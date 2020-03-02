/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "actions_p.h"
#include "validation.h"

#include "../base32/base32.h"
#include "../logging_p.h"
#include "../oath/oath.h"

#include <QScopedPointer>
#include <QTimer>

KEYSMITH_LOGGER(logger, ".accounts.actions")
KEYSMITH_LOGGER(dispatcherLogger, ".accounts.dispatcher")

namespace accounts
{
    AccountJob::AccountJob() : QObject()
    {
    }

    AccountJob::~AccountJob()
    {
    }

    Null::Null() : AccountJob()
    {
    }

    void Null::run(void)
    {
        Q_EMIT finished();
    }

    void AccountJob::run(void)
    {
        Q_ASSERT_X(false, Q_FUNC_INFO, "should be overridden in derived classes!");
    }

    RequestAccountPassword::RequestAccountPassword(const SettingsProvider &settings, AccountSecret *secret) : AccountJob(), m_settings(settings), m_secret(secret), m_failed(false), m_succeeded(false)
    {
    }

    LoadAccounts::LoadAccounts(const SettingsProvider &settings, const AccountSecret *secret) : AccountJob(), m_settings(settings), m_secret(secret)
    {
    }

    DeleteAccounts::DeleteAccounts(const SettingsProvider &settings, const QSet<QUuid> &ids) : AccountJob(), m_settings(settings), m_ids(ids)
    {
    }

    SaveHotp::SaveHotp(const SettingsProvider &settings, const QUuid &id, const QString &accountName, const secrets::EncryptedSecret &secret, quint64 counter, int tokenLength) :
        AccountJob(), m_settings(settings), m_id(id), m_accountName(accountName), m_secret(secret), m_counter(counter), m_tokenLength(tokenLength)
    {
    }

    SaveTotp::SaveTotp(const SettingsProvider &settings, const QUuid &id, const QString &accountName, const secrets::EncryptedSecret &secret, uint timeStep, int tokenLength) :
        AccountJob(), m_settings(settings), m_id(id), m_accountName(accountName), m_secret(secret), m_timeStep(timeStep), m_tokenLength(tokenLength)
    {
    }

    void SaveHotp::run(void)
    {
        if (!checkId(m_id) || !checkName(m_accountName) || !checkTokenLength(m_tokenLength)) {
            qCDebug(logger)
                << "Unable to save HOTP account:" << m_id
                << "Invalid account details";
            Q_EMIT invalid();
            Q_EMIT finished();
            return;
        }

        const PersistenceAction act([this](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                qCWarning(logger)
                    << "Unable to save HOTP account:" << m_id
                    << "Storage not writable";
                Q_EMIT invalid();
                return;
            }

            qCInfo(logger) << "Saving HOTP account:" << m_id;

            const QString group = m_id.toString();
            settings.remove(group);
            settings.beginGroup(group);
            settings.setValue("account", m_accountName);
            settings.setValue("type", "hotp");
            QString encodedNonce = QString::fromUtf8(m_secret.nonce().toBase64(QByteArray::Base64Encoding));
            QString encodedSecret = QString::fromUtf8(m_secret.cryptText().toBase64(QByteArray::Base64Encoding));
            settings.setValue("secret", encodedSecret);
            settings.setValue("nonce", encodedNonce);
            settings.setValue("counter", m_counter);
            settings.setValue("pinLength", m_tokenLength);
            settings.endGroup();

            // Try to guarantee that data will have been written before claiming the account was actually saved
            settings.sync();

            Q_EMIT saved(m_id, m_accountName, m_secret.cryptText(), m_secret.nonce(), m_counter, m_tokenLength);
        });
        m_settings(act);

        Q_EMIT finished();
    }

    void SaveTotp::run(void)
    {
        if (!checkId(m_id) || !checkName(m_accountName) || !checkTokenLength(m_tokenLength) || !checkTimeStep(m_timeStep)) {
            qCDebug(logger)
                << "Unable to save TOTP account:" << m_id
                << "Invalid account details";
            Q_EMIT invalid();
            Q_EMIT finished();
            return;
        }

        const PersistenceAction act([this](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                qCWarning(logger)
                    << "Unable to save TOTP account:" << m_id
                    << "Storage not writable";
                Q_EMIT invalid();
                return;
            }

            qCInfo(logger) << "Saving TOTP account:" << m_id;

            const QString group = m_id.toString();
            settings.remove(group);
            settings.beginGroup(group);
            settings.setValue("account", m_accountName);
            settings.setValue("type", "totp");
            QString encodedNonce = QString::fromUtf8(m_secret.nonce().toBase64(QByteArray::Base64Encoding));
            QString encodedSecret = QString::fromUtf8(m_secret.cryptText().toBase64(QByteArray::Base64Encoding));
            settings.setValue("secret", encodedSecret);
            settings.setValue("nonce", encodedNonce);
            settings.setValue("timeStep", m_timeStep);
            settings.setValue("pinLength", m_tokenLength);
            settings.endGroup();

            // Try to guarantee that data will have been written before claiming the account was actually saved
            settings.sync();

            Q_EMIT saved(m_id, m_accountName, m_secret.cryptText(), m_secret.nonce(), m_timeStep, m_tokenLength);
        });
        m_settings(act);

        Q_EMIT finished();
    }

    void DeleteAccounts::run(void)
    {
        const PersistenceAction act([this](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                qCWarning(logger) << "Unable to delete accounts: storage not writable";
                Q_EMIT invalid();
                return;
            }

            qCInfo(logger) << "Deleting accounts";

            for (const QUuid &id : m_ids) {
                settings.remove(id.toString());
            }
        });
        m_settings(act);

        Q_EMIT finished();
    }

    void RequestAccountPassword::fail(void)
    {
        if (m_failed || m_succeeded) {
            qCDebug(logger) << "Suppressing 'failure' in unlocking accounts: already handled";
            return;
        }

        m_failed = true;
        QObject::disconnect(m_secret, &AccountSecret::requestsCancelled, this, &RequestAccountPassword::fail);
        QObject::disconnect(m_secret, &AccountSecret::passwordAvailable, this, &RequestAccountPassword::unlock);
        Q_EMIT failed();
        Q_EMIT finished();
    }

    void RequestAccountPassword::unlock(void)
    {
        if (m_succeeded || m_failed) {
            qCDebug(logger) << "Suppressing 'success' in unlocking accounts: already handled";
            return;
        }

        QObject::disconnect(m_secret, &AccountSecret::requestsCancelled, this, &RequestAccountPassword::fail);
        QObject::disconnect(m_secret, &AccountSecret::passwordAvailable, this, &RequestAccountPassword::unlock);
        secrets::SecureMasterKey * derived = m_secret->deriveKey();
        if (!derived) {
            qCInfo(logger) << "Failed to unlock storage: unable to derive secret encryption/decryption key";
            m_failed = true;
            Q_EMIT failed();
            Q_EMIT finished();
            return;
        }

        bool ok = false;
        m_settings([this, derived, &ok](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                qCWarning(logger) << "Unable to save account secret key parameters: storage not writable";
                return;
            }

            const secrets::KeyDerivationParameters params = derived->params();

            QString encodedSalt = QString::fromUtf8(derived->salt().toBase64(QByteArray::Base64Encoding));
            settings.beginGroup("master-key");
            settings.setValue("salt", encodedSalt);
            settings.setValue("cpu", params.cpuCost());
            settings.setValue("memory", (quint64) params.memoryCost());
            settings.setValue("algorithm", params.algorithm());
            settings.setValue("length", params.keyLength());
            settings.endGroup();
            ok = true;
        });

        if (ok) {
            qCInfo(logger) << "Successfully unlocked storage";
            m_succeeded = true;
            Q_EMIT unlocked();
        } else {
            qCInfo(logger) << "Failed to unlock storage: unable to store parameters";
            m_failed = true;
            Q_EMIT failed();
        }
        Q_EMIT finished();
    }

    void RequestAccountPassword::run(void)
    {
        if (!m_secret) {
            qCDebug(logger) << "Unable to request accounts password: no account secret object";
            m_failed = true;
            Q_EMIT failed();
            Q_EMIT finished();
            return;
        }

        QObject::connect(m_secret, &AccountSecret::passwordAvailable, this, &RequestAccountPassword::unlock);
        QObject::connect(m_secret, &AccountSecret::requestsCancelled, this, &RequestAccountPassword::fail);

        if (!m_secret->isStillAlive()) {
            qCDebug(logger) << "Unable to request accounts password: account secret marked for death";
            fail();
            return;
        }

        bool ok = false;
        m_settings([this, &ok](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                qCWarning(logger) << "Unable to request password for accounts: storage not writable";
                return;
            }

            QStringList groups = settings.childGroups();
            if (!groups.contains(QLatin1String("master-key"))) {
                qCInfo(logger) << "No key derivation parameters found: requesting 'new' password for accounts";
                ok = m_secret->requestNewPassword();
                return;
            }

            settings.beginGroup("master-key");
            QByteArray salt;
            quint64 cpuCost = 0ULL;
            quint64 keyLength = 0ULL;
            size_t memoryCost = 0ULL;
            int algorithm = settings.value("algorithm").toInt(&ok);
            if (ok) {
                ok = false;
                keyLength = settings.value("length").toULongLong(&ok);
            }
            if (ok) {
                ok = false;
                cpuCost = settings.value("cpu").toULongLong(&ok);
            }
            if (ok) {
                ok = false;
                memoryCost = settings.value("memory").toULongLong(&ok);
            }
            if (ok) {
                QByteArray encodedSalt = settings.value("salt").toString().toUtf8();
                salt = QByteArray::fromBase64(encodedSalt, QByteArray::Base64Encoding);
                ok = secrets::SecureMasterKey::validate(salt);
            }
            settings.endGroup();

            std::optional<secrets::KeyDerivationParameters> params = secrets::KeyDerivationParameters::create(keyLength, algorithm, memoryCost, cpuCost);
            if (!ok || !params || !secrets::SecureMasterKey::validate(*params)) {
                qCDebug(logger) << "Unable to request 'existing' password: invalid salt or key derivation parameters";
                return;
            }

            qCInfo(logger) << "Requesting 'existing' password for accounts";
            ok = m_secret->requestExistingPassword(salt, *params);
        });

        if (!ok) {
            qCInfo(logger) << "Unable to unlock storage: failed to request password for accounts";
            fail();
        }
    }

    void LoadAccounts::run(void)
    {
        if (!m_secret || !m_secret->key()) {
            qCDebug(logger) << "Unable to load accounts: secret decryption key not available";
            Q_EMIT finished();
            return;
        }

        const PersistenceAction act([this](QSettings &settings) -> void
        {
            qCInfo(logger, "Loading accounts from storage");
            const QStringList entries = settings.childGroups();
            for (const QString &group : entries) {
                if (group == QLatin1String("master-key")) {
                    continue;
                }

                const QUuid id(group);
                if (id.isNull()) {
                    qCDebug(logger)
                        << "Ignoring:" << group
                        << "Not an account section";
                    continue;
                }

                settings.beginGroup(group);

                const QString accountName = settings.value("account").toString();
                if (!checkName(accountName)) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid account name";
                    settings.endGroup();
                    continue;
                }

                const QString type = settings.value("type", "hotp").toString();
                if (type != QLatin1String("hotp") && type != QLatin1String("totp")) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid account type";
                    settings.endGroup();
                    continue;
                }

                bool ok = false;
                const int tokenLength = settings.value("pinLength").toInt(&ok);
                if (!ok || !checkTokenLength(tokenLength)) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid token length";
                    settings.endGroup();
                    continue;
                }

                const QByteArray encodedNonce = settings.value("nonce").toString().toUtf8();
                const QByteArray encodedSecret = settings.value("secret").toString().toUtf8();
                const QByteArray nonce = QByteArray::fromBase64(encodedNonce, QByteArray::Base64Encoding);
                const QByteArray secret = QByteArray::fromBase64(encodedSecret, QByteArray::Base64Encoding);

                std::optional<secrets::EncryptedSecret> encryptedSecret = secrets::EncryptedSecret::from(secret, nonce);
                if (!encryptedSecret) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid token secret";
                    settings.endGroup();
                    continue;
                }

                QScopedPointer<secrets::SecureMemory> decrypted(m_secret->decrypt(*encryptedSecret));
                if (!decrypted) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Unable to decrypt token secret";
                    settings.endGroup();
                    continue;
                }

                if (type == QLatin1String("totp")) {
                    ok = false;
                    const uint timeStep = settings.value("timeStep").toUInt(&ok);
                    if (!ok || !checkTimeStep(timeStep)) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid time step";
                        settings.endGroup();
                        continue;
                    }

                    qCInfo(logger) << "Found valid TOTP account:" << id;
                    Q_EMIT foundTotp(id, accountName, secret, nonce, timeStep, tokenLength);
                }

                if (type == QLatin1String("hotp")) {
                    ok = false;
                    const quint64 counter = settings.value("counter").toULongLong(&ok);
                    if (!ok) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid counter";
                        settings.endGroup();
                        continue;
                    }

                    qCInfo(logger) << "Found valid HOTP account:" << id;
                    Q_EMIT foundHotp(id, accountName, secret, nonce, counter, tokenLength);
                }

                settings.endGroup();
            }
        });
        m_settings(act);

        Q_EMIT finished();
    }

    ComputeTotp::ComputeTotp(const AccountSecret *secret, const secrets::EncryptedSecret &tokenSecret, const QDateTime &epoch, uint timeStep, int tokenLength, const Account::Hash &hash, const std::function<qint64(void)> &clock) :
        AccountJob(), m_secret(secret), m_tokenSecret(tokenSecret), m_epoch(epoch), m_timeStep(timeStep), m_tokenLength(tokenLength), m_hash(hash), m_clock(clock)
    {
    }

    void ComputeTotp::run(void)
    {
        if (!m_secret || !m_secret->key()) {
            qCDebug(logger) << "Unable to compute TOTP token: secret decryption key not available";
            Q_EMIT finished();
            return;
        }

        if (!checkTokenLength(m_tokenLength)) {
            qCDebug(logger) << "Unable to compute THOTP token: invalid token length:" << m_tokenLength;
            Q_EMIT finished();
            return;
        }

        if (!checkTimeStep(m_timeStep)) {
            qCDebug(logger) << "Unable to compute THOTP token: invalid time step:" << m_timeStep;
            Q_EMIT finished();
            return;
        }

        QCryptographicHash::Algorithm hash;
        switch(m_hash)
        {
        case Account::Hash::Sha256:
            hash = QCryptographicHash::Sha256;
            break;
        case Account::Hash::Sha512:
            hash = QCryptographicHash::Sha512;
            break;
        case Account::Hash::Default:
            hash = QCryptographicHash::Sha1;
            break;
        default:
            qCDebug(logger) << "Unable to compute TOTP token: unknown hashing algorithm:" << m_hash;
            Q_EMIT finished();
            return;

        }

        const std::optional<oath::Algorithm> algorithm = oath::Algorithm::usingDynamicTruncation(hash, m_tokenLength);
        if (!algorithm) {
            qCDebug(logger) << "Unable to compute TOTP token: failed to construct algorithm";
            Q_EMIT finished();
            return;
        }

        const std::optional<quint64> counter = oath::count(m_epoch, m_timeStep, m_clock);
        if (!counter) {
            qCDebug(logger) << "Unable to compute TOTP token: failed to count time steps";
            Q_EMIT finished();
            return;
        }

        QScopedPointer<secrets::SecureMemory> secret(m_secret->decrypt(m_tokenSecret));
        if (!secret) {
            qCDebug(logger) << "Unable to compute TOTP token: failed to decrypt secret";
            Q_EMIT finished();
            return;
        }

        const std::optional<QString> token = algorithm->compute(*counter, reinterpret_cast<char*>(secret->data()), secret->size());
        if (token) {
            Q_EMIT otp(*token);
        } else {
            qCDebug(logger) << "Failed to compute TOTP token";
        }

        Q_EMIT finished();
    }

    ComputeHotp::ComputeHotp(const AccountSecret *secret, const secrets::EncryptedSecret &tokenSecret, quint64 counter, int tokenLength, int offset, bool checksum) :
        AccountJob(), m_secret(secret), m_tokenSecret(tokenSecret), m_counter(counter), m_tokenLength(tokenLength), m_offset(offset), m_checksum(checksum)
    {
    }

    void ComputeHotp::run(void)
    {
        if (!m_secret || !m_secret->key()) {
            qCDebug(logger) << "Unable to compute HOTP token: secret decryption key not available";
            Q_EMIT finished();
            return;
        }

        if (!checkTokenLength(m_tokenLength)) {
            qCDebug(logger) << "Unable to compute HOTP token: invalid token length:" << m_tokenLength;
            Q_EMIT finished();
            return;
        }

        const oath::Encoder encoder(m_tokenLength, m_checksum);
        const std::optional<oath::Algorithm> algorithm = m_offset >=0
            ? oath::Algorithm::usingTruncationOffset(QCryptographicHash::Sha1, (uint) m_offset, encoder)
            : oath::Algorithm::usingDynamicTruncation(QCryptographicHash::Sha1, encoder);
        if (!algorithm) {
            qCDebug(logger) << "Unable to compute HOTP token: failed to construct algorithm";
            Q_EMIT finished();
            return;
        }

        QScopedPointer<secrets::SecureMemory> secret(m_secret->decrypt(m_tokenSecret));
        if (!secret) {
            qCDebug(logger) << "Unable to compute HOTP token: failed to decrypt secret";
            Q_EMIT finished();
            return;
        }

        const std::optional<QString> token = algorithm->compute(m_counter, reinterpret_cast<char*>(secret->data()), secret->size());
        if (token) {
            Q_EMIT otp(*token);
        } else {
            qCDebug(logger) << "Failed to compute HOTP token";
        }

        Q_EMIT finished();
    }

    Dispatcher::Dispatcher(QThread *thread, QObject *parent) : QObject(parent), m_thread(thread),  m_current(nullptr)
    {
    }

    bool Dispatcher::empty(void) const
    {
        return m_pending.isEmpty();
    }

    void Dispatcher::queueAndProceed(AccountJob *job, const std::function<void(void)> &setup_callbacks)
    {
        if (job) {
            qCDebug(dispatcherLogger) << "Queuing job for dispatcher";
            job->moveToThread(m_thread);
            setup_callbacks();
            m_pending.append(job);
            dispatchNext();
        }
    }

    void Dispatcher::dispatchNext(void)
    {
        qCDebug(dispatcherLogger) << "Handling request to dispatch next job";

        if (!empty() && !m_current) {
            qCDebug(dispatcherLogger) << "Dispatching next job";

            m_current = m_pending.takeFirst();
            QObject::connect(m_current, &AccountJob::finished, this, &Dispatcher::next);
            QObject::connect(this, &Dispatcher::dispatch, m_current, &AccountJob::run);
            Q_EMIT dispatch();
        }
    }

    void Dispatcher::next(void)
    {
        qCDebug(dispatcherLogger) << "Handling next continuation in dispatcher";

        QObject *from = sender();
        AccountJob *job = from ? qobject_cast<AccountJob*>(from) : nullptr;
        if (job) {
            Q_ASSERT_X(job == m_current, Q_FUNC_INFO, "sender() should match 'current' job!");
            QObject::disconnect(this, &Dispatcher::dispatch, job, &AccountJob::run);
            QTimer::singleShot(0, job, &AccountJob::deleteLater);
            m_current = nullptr;
            dispatchNext();
        }
    }
}
