/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "actions_p.h"
#include "validation.h"

#include "../base32/base32.h"
#include "../logging_p.h"
#include "../oath/oath.h"

#include <QMetaEnum>
#include <QScopedPointer>
#include <QTimer>

#include <limits>

KEYSMITH_LOGGER(logger, ".accounts.actions")
KEYSMITH_LOGGER(dispatcherLogger, ".accounts.dispatcher")

static const quint64 maxCounter = std::numeric_limits<quint64>::max();
static const int hashTypeId = qRegisterMetaType<accounts::Account::Hash>();

namespace accounts
{
    AccountJob::AccountJob() :
        QObject()
    {
    }

    AccountJob::~AccountJob()
    {
    }

    Null::Null() :
        AccountJob()
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

    RequestAccountPassword::RequestAccountPassword(const SettingsProvider &settings, AccountSecret *secret) :
        AccountJob(), m_settings(settings), m_secret(secret), m_failed(false), m_succeeded(false)
    {
    }

    LoadAccounts::LoadAccounts(const SettingsProvider &settings, const AccountSecret *secret,
                               const std::function<qint64(void)> &clock) :
        AccountJob(), m_settings(settings), m_secret(secret), m_clock(clock)
    {
    }

    DeleteAccounts::DeleteAccounts(const SettingsProvider &settings, const QSet<QUuid> &ids) :
        AccountJob(), m_settings(settings), m_ids(ids)
    {
    }

    SaveHotp::SaveHotp(const SettingsProvider &settings,
                       const QUuid id, const QString &accountName, const QString &issuer,
                       const secrets::EncryptedSecret &secret, uint tokenLength,
                       quint64 counter, const std::optional<uint> offset, bool checksum) :
        AccountJob(), m_settings(settings), m_id(id), m_accountName(accountName), m_issuer(issuer),
        m_secret(secret), m_tokenLength(tokenLength), m_counter(counter), m_offset(offset), m_checksum(checksum)
    {
    }

    SaveTotp::SaveTotp(const SettingsProvider &settings,
                       const QUuid id, const QString &accountName, const QString &issuer,
                       const secrets::EncryptedSecret &secret, uint tokenLength,
                       uint timeStep, const QDateTime &epoch, Account::Hash hash,
                       const std::function<qint64(void)> &clock) :
        AccountJob(), m_settings(settings), m_id(id), m_accountName(accountName), m_issuer(issuer),
        m_secret(secret), m_tokenLength(tokenLength), m_timeStep(timeStep), m_epoch(epoch), m_hash(hash), m_clock(clock)
    {
    }

    void SaveHotp::run(void)
    {
        if (!checkId(m_id) || !checkName(m_accountName) || !checkIssuer(m_issuer) ||
            !checkTokenLength(m_tokenLength) || !checkOffset(m_offset, QCryptographicHash::Sha1)) {
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
            settings.setValue(QStringLiteral("account"), m_accountName);
            if (!m_issuer.isNull()) {
                settings.setValue(QStringLiteral("issuer"), m_issuer);
            }
            settings.setValue(QStringLiteral("type"), QStringLiteral("hotp"));
            QString encodedNonce = QString::fromUtf8(m_secret.nonce().toBase64(QByteArray::Base64Encoding));
            QString encodedSecret = QString::fromUtf8(m_secret.cryptText().toBase64(QByteArray::Base64Encoding));
            settings.setValue(QStringLiteral("secret"), encodedSecret);
            settings.setValue(QStringLiteral("nonce"), encodedNonce);
            settings.setValue(QStringLiteral("counter"), m_counter);
            settings.setValue(QStringLiteral("pinLength"), m_tokenLength);
            if (m_offset) {
                settings.setValue(QStringLiteral("offset"), *m_offset);
            }
            settings.setValue(QStringLiteral("checksum"), m_checksum);
            settings.endGroup();

            // Try to guarantee that data will have been written before claiming the account was actually saved
            settings.sync();

            Q_EMIT saved(m_id, m_accountName, m_issuer, m_secret.cryptText(), m_secret.nonce(), m_tokenLength,
                         m_counter, m_offset.has_value(), m_offset ? *m_offset : 0U, m_checksum);
        });
        m_settings(act);

        Q_EMIT finished();
    }

    void SaveTotp::run(void)
    {
        if (!checkId(m_id) || !checkName(m_accountName) || !checkIssuer(m_issuer) ||
            !checkTokenLength(m_tokenLength) || !checkTimeStep(m_timeStep) || !checkEpoch(m_epoch, m_clock)) {
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
            settings.setValue(QStringLiteral("account"), m_accountName);
            if (!m_issuer.isNull()) {
                settings.setValue(QStringLiteral("issuer"), m_issuer);
            }
            settings.setValue(QStringLiteral("type"), QStringLiteral("totp"));
            QString encodedNonce = QString::fromUtf8(m_secret.nonce().toBase64(QByteArray::Base64Encoding));
            QString encodedSecret = QString::fromUtf8(m_secret.cryptText().toBase64(QByteArray::Base64Encoding));
            settings.setValue(QStringLiteral("secret"), encodedSecret);
            settings.setValue(QStringLiteral("nonce"), encodedNonce);
            settings.setValue(QStringLiteral("timeStep"), m_timeStep);
            settings.setValue(QStringLiteral("pinLength"), m_tokenLength);
            settings.setValue(QStringLiteral("epoch"), m_epoch.toUTC().toString(Qt::ISODateWithMs));
            settings.setValue(QStringLiteral("hash"), QVariant::fromValue<Account::Hash>(m_hash).toString());
            settings.endGroup();

            // Try to guarantee that data will have been written before claiming the account was actually saved
            settings.sync();

            Q_EMIT saved(m_id, m_accountName, m_issuer, m_secret.cryptText(), m_secret.nonce(), m_tokenLength,
                         m_timeStep, m_epoch, m_hash);
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
        QObject::disconnect(m_secret, &AccountSecret::keyAvailable, this, &RequestAccountPassword::finish);
        Q_EMIT failed();
        Q_EMIT finished();
    }

    void RequestAccountPassword::unlock(void)
    {
        secrets::SecureMasterKey * derived = m_secret->deriveKey();
        std::optional<secrets::EncryptedSecret> challenge = m_secret->challenge();
        if (derived && challenge) {
            qCInfo(logger) << "Successfully derived key for storage";
            return;
        } else {
            qCInfo(logger) << "Failed to unlock storage:"
                << "Unable to derive secret encryption/decryption key or generate its matching challenge";
        }
    }

    void RequestAccountPassword::finish(void)
    {
        if (m_succeeded || m_failed) {
            qCDebug(logger) << "Suppressing 'success' in unlocking accounts: already handled";
            return;
        }

        QObject::disconnect(m_secret, &AccountSecret::requestsCancelled, this, &RequestAccountPassword::fail);
        QObject::disconnect(m_secret, &AccountSecret::passwordAvailable, this, &RequestAccountPassword::unlock);
        QObject::disconnect(m_secret, &AccountSecret::keyAvailable, this, &RequestAccountPassword::finish);
        std::optional<secrets::EncryptedSecret> challenge = m_secret->challenge();
        secrets::SecureMasterKey * derived = m_secret->key();
        if (!derived) {
            qCInfo(logger) << "Failed to finish unlocking storage: no secret encryption/decryption key";
            m_failed = true;
            Q_EMIT failed();
            Q_EMIT finished();
            return;
        }

        // sanity check: challenge should be available once key derivation has completed successfully
        if (!challenge) {
            qCInfo(logger) << "Failed to finish unlocking storage: no challenge for encryption/decryption key";
            m_failed = true;
            Q_EMIT failed();
            Q_EMIT finished();
            return;
        }

        bool ok = false;
        m_settings([derived, &challenge, &ok](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                qCWarning(logger) << "Unable to save account secret key parameters: storage not writable";
                return;
            }

            const secrets::KeyDerivationParameters params = derived->params();

            QString encodedSalt = QString::fromUtf8(derived->salt().toBase64(QByteArray::Base64Encoding));
            QString encodedChallenge = QString::fromUtf8(challenge->cryptText().toBase64(QByteArray::Base64Encoding));
            QString encodedNonce = QString::fromUtf8(challenge->nonce().toBase64(QByteArray::Base64Encoding));
            settings.beginGroup(QStringLiteral("master-key"));
            settings.setValue(QStringLiteral("salt"), encodedSalt);
            settings.setValue(QStringLiteral("cpu"), params.cpuCost());
            settings.setValue(QStringLiteral("memory"), (quint64) params.memoryCost());
            settings.setValue(QStringLiteral("algorithm"), params.algorithm());
            settings.setValue(QStringLiteral("length"), params.keyLength());
            settings.setValue(QStringLiteral("nonce"), encodedNonce);
            settings.setValue(QStringLiteral("challenge"), encodedChallenge);
            settings.endGroup();
            ok = true;
        });

        if (ok) {
            qCInfo(logger) << "Successfully unlocked storage";
            m_succeeded = true;
            Q_EMIT unlocked();
        } else {
            qCInfo(logger) << "Failed to finish unlocking storage: unable to store parameters";
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
        QObject::connect(m_secret, &AccountSecret::keyAvailable, this, &RequestAccountPassword::finish);

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
            if (!groups.contains(QStringLiteral("master-key"))) {
                qCInfo(logger) << "No key derivation parameters found: requesting 'new' password for accounts";
                ok = m_secret->requestNewPassword();
                return;
            }

            settings.beginGroup(QStringLiteral("master-key"));
            QByteArray salt;
            QByteArray nonce;
            QByteArray challenge;
            quint64 cpuCost = 0ULL;
            quint64 keyLength = 0ULL;
            size_t memoryCost = 0ULL;
            int algorithm = settings.value(QStringLiteral("algorithm")).toInt(&ok);
            if (ok) {
                ok = false;
                keyLength = settings.value(QStringLiteral("length")).toULongLong(&ok);
            }
            if (ok) {
                ok = false;
                cpuCost = settings.value(QStringLiteral("cpu")).toULongLong(&ok);
            }
            if (ok) {
                ok = false;
                memoryCost = settings.value(QStringLiteral("memory")).toULongLong(&ok);
            }
            if (ok) {
                QByteArray encodedSalt = settings.value(QStringLiteral("salt")).toString().toUtf8();
                salt = QByteArray::fromBase64(encodedSalt, QByteArray::Base64Encoding);
                ok = !salt.isEmpty() && secrets::SecureMasterKey::validate(salt);
            }
            if (ok) {
                QByteArray encodedChallenge = settings.value(QStringLiteral("challenge")).toString().toUtf8();
                challenge = QByteArray::fromBase64(encodedChallenge, QByteArray::Base64Encoding);
                ok = !challenge.isEmpty();
            }
            if (ok) {
                QByteArray encodedNonce = settings.value(QStringLiteral("nonce")).toString().toUtf8();
                nonce = QByteArray::fromBase64(encodedNonce, QByteArray::Base64Encoding);
                ok = !nonce.isEmpty();
            }
            settings.endGroup();

            const auto params = secrets::KeyDerivationParameters::create(keyLength, algorithm, memoryCost, cpuCost);
            const auto encryptedChallenge = secrets::EncryptedSecret::from(challenge, nonce);
            if (!ok || !params || !secrets::SecureMasterKey::validate(*params) || !encryptedChallenge) {
                qCDebug(logger) << "Unable to request 'existing' password: invalid challenge, nonce, salt or key derivation parameters";
                return;
            }

            qCInfo(logger) << "Requesting 'existing' password for accounts";
            ok = m_secret->requestExistingPassword(*encryptedChallenge, salt, *params);
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

        bool failed = false;
        const PersistenceAction act([this, &failed](QSettings &settings) -> void
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
                    failed = true;
                    continue;
                }

                settings.beginGroup(group);

                const QString accountName = settings.value(QStringLiteral("account")).toString();
                if (!checkName(accountName)) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid account name";
                    settings.endGroup();
                    continue;
                }

                const QString issuer = settings.value(QStringLiteral("issuer"), QString()).toString();
                if (!checkIssuer(issuer)) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid account issuer";
                    settings.endGroup();
                    continue;
                }

                const QString type = settings.value(QStringLiteral("type")).toString();
                if (type != QStringLiteral("hotp") && type != QStringLiteral("totp")) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid account type";
                    settings.endGroup();
                    failed = true;
                    continue;
                }

                bool ok = false;
                const int tokenLength = settings.value(QStringLiteral("pinLength")).toInt(&ok);
                if (!ok || !checkTokenLength(tokenLength)) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid token length";
                    settings.endGroup();
                    failed = true;
                    continue;
                }

                const QByteArray encodedNonce = settings.value(QStringLiteral("nonce")).toString().toUtf8();
                const QByteArray encodedSecret = settings.value(QStringLiteral("secret")).toString().toUtf8();
                const QByteArray nonce = QByteArray::fromBase64(encodedNonce, QByteArray::Base64Encoding);
                const QByteArray secret = QByteArray::fromBase64(encodedSecret, QByteArray::Base64Encoding);

                const auto encryptedSecret = secrets::EncryptedSecret::from(secret, nonce);
                if (!encryptedSecret) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Invalid token secret";
                    settings.endGroup();
                    failed = true;
                    continue;
                }

                QScopedPointer<secrets::SecureMemory> decrypted(m_secret->decrypt(*encryptedSecret));
                if (!decrypted) {
                    qCWarning(logger)
                        << "Skipping invalid account:" << id
                        << "Unable to decrypt token secret";
                    settings.endGroup();
                    failed = true;
                    continue;
                }

                if (type == QStringLiteral("totp")) {
                    ok = false;
                    const uint timeStep = settings.value(QStringLiteral("timeStep")).toUInt(&ok);
                    if (!ok || !checkTimeStep(timeStep)) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid time step";
                        settings.endGroup();
                        failed = true;
                        continue;
                    }

                    const QDateTime epoch = settings.value(QStringLiteral("epoch"), QDateTime::fromMSecsSinceEpoch(0))
                        .toDateTime();
                    if (!checkEpoch(epoch, m_clock)) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid epoch";
                        settings.endGroup();
                        failed = true;
                        continue;
                    }

                    ok = false;

                    const auto hashEnum = QMetaEnum::fromType<accounts::Account::Hash>();
                    const auto hashDefault = QVariant::fromValue<accounts::Account::Hash>(accounts::Account::Sha1);
                    const QByteArray hashName = settings.value(QStringLiteral("hash"), hashDefault).toByteArray();
                    int hash = hashEnum.keyToValue(hashName.constData(), &ok);
                    if (!ok) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid hash";
                        settings.endGroup();
                        failed = true;
                        continue;
                    }

                    qCInfo(logger) << "Found valid TOTP account:" << id;
                    Q_EMIT foundTotp(id, accountName, issuer, secret, nonce, tokenLength,
                                     timeStep, epoch, (Account::Hash) hash);
                }

                if (type == QStringLiteral("hotp")) {
                    ok = false;
                    const quint64 counter = settings.value(QStringLiteral("counter")).toULongLong(&ok);
                    if (!ok) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid counter";
                        settings.endGroup();
                        failed = true;
                        continue;
                    }

                    const QVariant offsetVariant = settings.value(QStringLiteral("offset"));
                    ok = offsetVariant.isNull();
                    std::optional<uint> offset = ok ? std::nullopt : std::optional<uint>(offsetVariant.toUInt(&ok));

                    if (!ok || !checkOffset(offset, QCryptographicHash::Sha1)) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid offset";
                        settings.endGroup();
                        failed = true;
                        continue;
                    }

                    const auto checkSumOff = QStringLiteral("false");
                    const auto checksum = settings.value(QStringLiteral("checksum"), checkSumOff).toString();
                    if (checksum != QStringLiteral("true") && checksum != checkSumOff) {
                        qCWarning(logger)
                            << "Skipping invalid account:" << id
                            << "Invalid checksum";
                        settings.endGroup();
                        failed = true;
                        continue;
                    }

                    qCInfo(logger) << "Found valid HOTP account:" << id;
                    Q_EMIT foundHotp(id, accountName, issuer, secret, nonce, tokenLength,
                                     counter, offset.has_value(), offset ? *offset : 0U,
                                     checksum == QStringLiteral("true"));
                }

                settings.endGroup();
            }
        });
        m_settings(act);

        if (failed) {
            Q_EMIT failedToLoadAllAccounts();
        }
        Q_EMIT finished();
    }

    static std::optional<QString> computeToken(const AccountSecret *accountSecret,
                                               const secrets::EncryptedSecret &tokenSecret,
                                               const oath::Algorithm &algorithm,
                                               quint64 counter)
    {
        QScopedPointer<secrets::SecureMemory> secret(accountSecret->decrypt(tokenSecret));
        if (!secret) {
            qCDebug(logger) << "Unable to compute token: failed to decrypt account secret";
            return std::nullopt;
        }

        return algorithm.compute(counter, reinterpret_cast<char*>(secret->data()), secret->size());
    }


    ComputeTotp::ComputeTotp(const AccountSecret *secret,
                             const secrets::EncryptedSecret &tokenSecret, uint tokenLength,
                             const QDateTime &epoch, uint timeStep, const Account::Hash hash,
                             const std::function<qint64(void)> &clock) :
        AccountJob(), m_secret(secret), m_tokenSecret(tokenSecret), m_tokenLength(tokenLength),
        m_epoch(epoch), m_timeStep(timeStep), m_hash(hash), m_clock(clock)
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
            qCDebug(logger) << "Unable to compute TOTP token: invalid token length:" << m_tokenLength;
            Q_EMIT finished();
            return;
        }

        if (!checkTimeStep(m_timeStep)) {
            qCDebug(logger) << "Unable to compute TOTP token: invalid time step:" << m_timeStep;
            Q_EMIT finished();
            return;
        }

        if (!checkEpoch(m_epoch, m_clock)) {
            qCDebug(logger) << "Unable to compute TOTP token: invalid epoch:" << m_epoch;
            Q_EMIT finished();
            return;
        }

        QCryptographicHash::Algorithm hash;
        switch(m_hash)
        {
        case Account::Hash::Sha1:
            hash = QCryptographicHash::Sha1;
            break;
        case Account::Hash::Sha256:
            hash = QCryptographicHash::Sha256;
            break;
        case Account::Hash::Sha512:
            hash = QCryptographicHash::Sha512;
            break;
        default:
            qCDebug(logger) << "Unable to compute TOTP token: unknown hashing algorithm:" << m_hash;
            Q_EMIT finished();
            return;

        }

        const std::optional<oath::Algorithm> algorithm = oath::Algorithm::totp(hash, m_tokenLength);
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

        const auto counterValue = *counter;
        const auto validFrom = counterValue < maxCounter
            ? oath::fromCounter(counterValue + 1ULL, m_epoch, m_timeStep)
            : std::nullopt;
        const auto validUntil = counterValue < (maxCounter - 1ULL)
            ? oath::fromCounter(counterValue + 2ULL, m_epoch, m_timeStep)
            : std::nullopt;
        if (!validFrom || !validUntil) {
            qCDebug(logger) << "Unable to compute TOTP token: failed to determine expiry datetime of tokens";
            Q_EMIT finished();
            return;
        }

        const auto token = computeToken(m_secret, m_tokenSecret, *algorithm, counterValue);
        const auto nextToken = token
            ? computeToken(m_secret, m_tokenSecret, *algorithm, counterValue + 1ULL)
            : std::nullopt;
        if (token && nextToken) {
            Q_EMIT otp(*token, *nextToken, *validFrom, *validUntil);
        } else {
            qCDebug(logger) << "Failed to compute TOTP tokens";
        }

        Q_EMIT finished();
    }

    ComputeHotp::ComputeHotp(const AccountSecret *secret,
                             const secrets::EncryptedSecret &tokenSecret, uint tokenLength,
                             quint64 counter, const std::optional<uint> offset, bool checksum) :
        AccountJob(), m_secret(secret), m_tokenSecret(tokenSecret), m_tokenLength(tokenLength),
        m_counter(counter), m_offset(offset), m_checksum(checksum)
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

        if (!checkOffset(m_offset, QCryptographicHash::Sha1)) {
            qCDebug(logger) << "Unable to compute HOTP token: invalid offset:" << *m_offset;
            Q_EMIT finished();
            return;
        }

        const std::optional<oath::Algorithm> algorithm = oath::Algorithm::hotp(m_offset, m_tokenLength, m_checksum);
        if (!algorithm) {
            qCDebug(logger) << "Unable to compute HOTP token: failed to construct algorithm";
            Q_EMIT finished();
            return;
        }

        if (m_counter == maxCounter) {
            qCDebug(logger) << "Unable to compute HOTP token: counter reached its limit";
            Q_EMIT finished();
            return;
        }

        const auto token = computeToken(m_secret, m_tokenSecret, *algorithm, m_counter);
        const auto nextToken = token
            ? computeToken(m_secret, m_tokenSecret, *algorithm, m_counter + 1ULL)
            : std::nullopt;
        if (token && nextToken) {
            Q_EMIT otp(*token, *nextToken, m_counter + 1ULL);
        } else {
            qCDebug(logger) << "Failed to compute HOTP tokens";
        }

        Q_EMIT finished();
    }

    Dispatcher::Dispatcher(QThread *thread, QObject *parent) :
        QObject(parent), m_thread(thread),  m_current(nullptr)
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
