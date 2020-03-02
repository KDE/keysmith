/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_ACTIONS_P_H
#define ACCOUNTS_ACTIONS_P_H

#include "account.h"

#include <QObject>
#include <QDateTime>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QUuid>
#include <QVector>

#include <functional>

#include "../secrets/secrets.h"
#include "keys.h"

namespace accounts
{
    class AccountJob: public QObject
    {
        Q_OBJECT
    public:
        explicit AccountJob();
        virtual ~AccountJob();
    public Q_SLOTS:
        virtual void run(void);
    Q_SIGNALS:
        void finished(void);
    };

    class Null : public AccountJob
    {
        Q_OBJECT
    public:
        explicit Null();
        void run(void) override;
    };

    class RequestAccountPassword: public AccountJob
    {
        Q_OBJECT
    public:
        explicit RequestAccountPassword(const SettingsProvider &settings, AccountSecret *secret);
        void run(void) override;
    private Q_SLOTS:
        void fail(void);
        void unlock(void);
    Q_SIGNALS:
        void unlocked(void);
        void failed(void);
    private:
        const SettingsProvider m_settings;
        AccountSecret * m_secret;
    private:
        bool m_failed;
        bool m_succeeded;
    };

    class LoadAccounts: public AccountJob
    {
        Q_OBJECT
    public:
        explicit LoadAccounts(const SettingsProvider &settings, const AccountSecret *secret);
        void run(void) override;
    Q_SIGNALS:
        void foundHotp(const QUuid id, const QString name, const QByteArray secret, const QByteArray nonce, quint64 counter, int tokenLength);
        void foundTotp(const QUuid id, const QString name, const QByteArray secret, const QByteArray nonce, uint timeStep, int tokenLength);
    private:
        const SettingsProvider m_settings;
        const AccountSecret * m_secret;
    };

    class DeleteAccounts: public AccountJob
    {
        Q_OBJECT
    public:
        explicit DeleteAccounts(const SettingsProvider &settings, const QSet<QUuid> &ids);
        void run(void) override;
    Q_SIGNALS:
        void invalid(void);
    private:
        const SettingsProvider m_settings;
        const QSet<QUuid> m_ids;
    };

    class SaveHotp: public AccountJob
    {
        Q_OBJECT
    public:
        explicit SaveHotp(const SettingsProvider &settings, const QUuid &id, const QString &accountName, const secrets::EncryptedSecret &secret, quint64 counter, int tokenLength);
        void run(void) override;
    Q_SIGNALS:
        void invalid(void);
        void saved(const QUuid id, const QString accountName, const QByteArray secret, const QByteArray nonce, quint64 counter, int tokenLength);
    private:
        const SettingsProvider m_settings;
        const QUuid m_id;
        const QString m_accountName;
        const secrets::EncryptedSecret m_secret;
        const quint64 m_counter;
        const int m_tokenLength;
    };

    class SaveTotp: public AccountJob
    {
        Q_OBJECT
    public:
        explicit SaveTotp(const SettingsProvider &settings, const QUuid &id, const QString &accountName, const secrets::EncryptedSecret &secret, uint timeStep, int tokenLength);
        void run(void) override;
    Q_SIGNALS:
        void invalid(void);
        void saved(const QUuid id, const QString accountName, const QByteArray secret, const QByteArray nonce, uint timeStep, int tokenLength);
    private:
        const SettingsProvider m_settings;
        const QUuid m_id;
        const QString m_accountName;
        const secrets::EncryptedSecret m_secret;
        const uint m_timeStep;
        const int m_tokenLength;
    };

    class ComputeTotp: public AccountJob
    {
        Q_OBJECT
    public:
        explicit ComputeTotp(const AccountSecret *secret, const secrets::EncryptedSecret &tokenSecret, const QDateTime &epoch, uint timeStep, int tokenLength, const Account::Hash &hash = Account::Hash::Default, const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);
        void run(void) override;
    Q_SIGNALS:
        void otp(const QString otp);
    private:
        const AccountSecret * m_secret;
        const secrets::EncryptedSecret m_tokenSecret;
        const QDateTime m_epoch;
        const uint m_timeStep;
        const int m_tokenLength;
        const Account::Hash m_hash;
        const std::function<qint64(void)> m_clock;
    };

    class ComputeHotp: public AccountJob
    {
        Q_OBJECT
    public:
        explicit ComputeHotp(const AccountSecret *secret, const secrets::EncryptedSecret &tokenSecret, quint64 counter, int tokenLength, int offset = -1, bool checksum = false);
        void run(void) override;
    Q_SIGNALS:
        void otp(const QString otp);
    private:
        const AccountSecret * m_secret;
        const secrets::EncryptedSecret m_tokenSecret;
        const quint64 m_counter;
        const int m_tokenLength;
        const int m_offset;
        const bool m_checksum;
    };

    class Dispatcher: public QObject
    {
        Q_OBJECT
    public:
        explicit Dispatcher(QThread *thread, QObject *parent = nullptr);
        bool empty(void) const;
        void queueAndProceed(AccountJob *job, const std::function<void(void)> &setup_callbacks);
    Q_SIGNALS:
        void dispatch(void);
    private Q_SLOTS:
        void next(void);
    private:
        void dispatchNext(void);
    private:
        QThread * const m_thread;
    private:
        AccountJob *m_current;
        QVector<AccountJob*> m_pending;
    };
}

#endif
