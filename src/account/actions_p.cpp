/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "actions_p.h"
#include "validation.h"

#include "../base32/base32.h"
#include "../oath/oath.h"

#include <QTimer>

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

    LoadAccounts::LoadAccounts(const SettingsProvider &settings) : AccountJob(), m_settings(settings)
    {
    }

    DeleteAccounts::DeleteAccounts(const SettingsProvider &settings, const QSet<QUuid> &ids) : AccountJob(), m_settings(settings), m_ids(ids)
    {
    }

    SaveHotp::SaveHotp(const SettingsProvider &settings, const QUuid &id, const QString &accountName, const QString &secret, quint64 counter, int tokenLength) :
        AccountJob(), m_settings(settings), m_id(id), m_accountName(accountName), m_secret(secret), m_counter(counter), m_tokenLength(tokenLength)
    {
    }

    SaveTotp::SaveTotp(const SettingsProvider &settings, const QUuid &id, const QString &accountName, const QString &secret, uint timeStep, int tokenLength) :
        AccountJob(), m_settings(settings), m_id(id), m_accountName(accountName), m_secret(secret), m_timeStep(timeStep), m_tokenLength(tokenLength)
    {
    }

    void SaveHotp::run(void)
    {
        if (!checkHotpAccount(m_id, m_accountName, m_secret, m_tokenLength)) {
            Q_EMIT invalid();
            Q_EMIT finished();
            return;
        }

        const PersistenceAction act([this](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                // TODO: warn if not
                Q_EMIT invalid();
                return;
            }

            const QString group = m_id.toString();
            settings.remove(group);
            settings.beginGroup(group);
            settings.setValue("account", m_accountName);
            settings.setValue("type", "hotp");
            settings.setValue("secret", m_secret);
            settings.setValue("counter", m_counter);
            settings.setValue("pinLength", m_tokenLength);
            settings.endGroup();

            // Try to guarantee that data will have been written before claiming the account was actually saved
            settings.sync();

            Q_EMIT saved(m_id, m_accountName, m_secret, m_counter, m_tokenLength);
        });
        m_settings(act);

        Q_EMIT finished();
    }

    void SaveTotp::run(void)
    {
        if (!checkTotpAccount(m_id, m_accountName, m_secret, m_tokenLength, m_timeStep)) {
            // TODO: warn if not
            Q_EMIT invalid();
            Q_EMIT finished();
            return;
        }

        const PersistenceAction act([this](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                // TODO: warn if not
                Q_EMIT invalid();
                return;
            }

            const QString group = m_id.toString();
            settings.remove(group);
            settings.beginGroup(group);
            settings.setValue("account", m_accountName);
            settings.setValue("type", "totp");
            settings.setValue("secret", m_secret);
            settings.setValue("timeStep", m_timeStep);
            settings.setValue("pinLength", m_tokenLength);
            settings.endGroup();

            // Try to guarantee that data will have been written before claiming the account was actually saved
            settings.sync();

            Q_EMIT saved(m_id, m_accountName, m_secret, m_timeStep, m_tokenLength);
        });
        m_settings(act);

        Q_EMIT finished();
    }

    void DeleteAccounts::run(void)
    {
        const PersistenceAction act([this](QSettings &settings) -> void
        {
            if (!settings.isWritable()) {
                // TODO: warn if not
                Q_EMIT invalid();
                return;
            }

            for (const QUuid &id : m_ids) {
                settings.remove(id.toString());
            }
        });
        m_settings(act);

        Q_EMIT finished();
    }

    void LoadAccounts::run(void)
    {
        const PersistenceAction act([this](QSettings &settings) -> void
        {
            const QStringList entries = settings.childGroups();
            for (const QString &group : entries) {
                const QUuid id(group);

                if (id.isNull()) {
                    continue;
                }

                bool ok = false;
                settings.beginGroup(group);

                const QString secret = settings.value("secret").toString();
                const QString accountName = settings.value("account").toString();
                const QString type = settings.value("type", "hotp").toString();
                const int tokenLength = settings.value("pinLength").toInt(&ok);

                if (!ok || (type != "hotp" && type != "totp")) {
                    continue;
                }

                if (type == "totp") {
                    ok = false;
                    const uint timeStep = settings.value("timeStep").toUInt(&ok);
                    if (ok && checkTotpAccount(id, accountName, secret, tokenLength, timeStep)) {
                        Q_EMIT foundTotp(
                            id,
                            accountName,
                            secret,
                            timeStep,
                            tokenLength
                        );
                    }
                }
                if (type == "hotp") {
                    ok = false;
                    const quint64 counter = settings.value("counter").toULongLong(&ok);
                    if (ok && checkHotpAccount(id, accountName, secret, tokenLength)) {
                        Q_EMIT foundHotp(
                            id,
                            accountName,
                            secret,
                            counter,
                            tokenLength
                        );
                    }
                }

                settings.endGroup();
            }
        });
        m_settings(act);

        Q_EMIT finished();
    }

    ComputeTotp::ComputeTotp(const QString &secret, const QDateTime &epoch, uint timeStep, int tokenLength, const Account::Hash &hash, const std::function<qint64(void)> &clock) :
        AccountJob(), m_secret(secret), m_epoch(epoch), m_timeStep(timeStep), m_tokenLength(tokenLength), m_hash(hash), m_clock(clock)
    {
    }

    void ComputeTotp::run(void)
    {
        if (!checkTotp(m_secret, m_tokenLength, m_timeStep)) {
            Q_EMIT finished();
            // TODO warn about this
            return;
        }

        std::optional<QByteArray> secret = base32::decode(m_secret);
        if (!secret.has_value()) {
            Q_EMIT finished();
            // TODO warn about this
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
            Q_EMIT finished();
            // TODO warn about this
            return;

        }

        const std::optional<oath::Algorithm> algorithm = oath::Algorithm::usingDynamicTruncation(hash, m_tokenLength);
        if (!algorithm) {
            Q_EMIT finished();
            // TODO warn about this
            return;
        }

        const std::optional<quint64> counter = oath::count(m_epoch, m_timeStep, m_clock);
        if (!counter) {
            Q_EMIT finished();
            // TODO warn about this
            return;
        }

        const std::optional<QString> token = algorithm->compute(*counter, secret->data(), secret->size());
        if (token) {
            Q_EMIT otp(*token);
        }
        // TODO warn if not

        Q_EMIT finished();
    }

    ComputeHotp::ComputeHotp(const QString &secret, quint64 counter, int tokenLength, int offset, bool checksum) :
        AccountJob(), m_secret(secret), m_counter(counter), m_tokenLength(tokenLength), m_offset(offset), m_checksum(checksum)
    {
    }

    void ComputeHotp::run(void)
    {
        if (!checkHotp(m_secret, m_tokenLength)) {
            Q_EMIT finished();
            // TODO warn about this
            return;
        }

        std::optional<QByteArray> secret = base32::decode(m_secret);
        if (!secret.has_value()) {
            Q_EMIT finished();
            // TODO warn about this
            return;
        }

        const oath::Encoder encoder(m_tokenLength, m_checksum);
        const std::optional<oath::Algorithm> algorithm = m_offset >=0
            ? oath::Algorithm::usingTruncationOffset(QCryptographicHash::Sha1, (uint) m_offset, encoder)
            : oath::Algorithm::usingDynamicTruncation(QCryptographicHash::Sha1, encoder);
        if (!algorithm) {
            Q_EMIT finished();
            // TODO warn about this
            return;
        }

        const std::optional<QString> token = algorithm->compute(m_counter, secret->data(), secret->size());
        if (token) {
            Q_EMIT otp(*token);
        }
        // TODO warn if not

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
            job->moveToThread(m_thread);
            setup_callbacks();
            m_pending.append(job);
            dispatchNext();
        }
    }

    void Dispatcher::dispatchNext(void)
    {
        if (!empty() && !m_current) {
            m_current = m_pending.takeFirst();
            QObject::connect(m_current, &AccountJob::finished, this, &Dispatcher::next);
            QObject::connect(this, &Dispatcher::dispatch, m_current, &AccountJob::run);
            Q_EMIT dispatch();
        }
    }

    void Dispatcher::next(void)
    {
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
