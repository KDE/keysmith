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
#include <optional>

#include "../secrets/secrets.h"
#include "keys.h"

namespace accounts
{
    class AccountJob: public QObject // clazy:exclude=ctor-missing-parent-argument
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

    class Null: public AccountJob // clazy:exclude=ctor-missing-parent-argument
    {
        Q_OBJECT
    public:
        explicit Null();
        void run(void) override;
    };

    class RequestAccountPassword: public AccountJob // clazy:exclude=ctor-missing-parent-argument
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
        AccountSecret * const m_secret;
    private:
        bool m_failed;
        bool m_succeeded;
    };

    class LoadAccounts: public AccountJob // clazy:exclude=ctor-missing-parent-argument
    {
        Q_OBJECT
    public:
        explicit LoadAccounts(const SettingsProvider &settings, const AccountSecret *secret,
                              const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);
        void run(void) override;
    Q_SIGNALS:
        void foundHotp(const QUuid id, const QString name, const QString issuer,
                       const QByteArray secret, const QByteArray nonce, uint tokenLength,
                       quint64 counter, bool fixedTruncation, uint offset, bool checksum);
        void foundTotp(const QUuid id, const QString name, const QString issuer,
                       const QByteArray secret, const QByteArray nonce, uint tokenLength,
                       uint timeStep, const QDateTime epoch, accounts::Account::Hash hash);
        void failedToLoadAllAccounts(void);
    private:
        const SettingsProvider m_settings;
        const AccountSecret * const m_secret;
        const std::function<qint64(void)> m_clock;
    };

    class DeleteAccounts: public AccountJob // clazy:exclude=ctor-missing-parent-argument
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

    class SaveHotp: public AccountJob // clazy:exclude=ctor-missing-parent-argument
    {
        Q_OBJECT
    public:
        explicit SaveHotp(const SettingsProvider &settings,
                          const QUuid id, const QString &accountName, const QString &issuer,
                          const secrets::EncryptedSecret &secret, uint tokenLength,
                          quint64 counter, const std::optional<uint> offset, bool checksum);
        void run(void) override;
    Q_SIGNALS:
        void invalid(void);
        void saved(const QUuid id, const QString accountName, const QString issuer,
                   const QByteArray secret, const QByteArray nonce, uint tokenLength,
                   quint64 counter, bool fixedTruncation, uint offset, bool checksum);
    private:
        const SettingsProvider m_settings;
        const QUuid m_id;
        const QString m_accountName;
        const QString m_issuer;
        const secrets::EncryptedSecret m_secret;
        const uint m_tokenLength;
        const quint64 m_counter;
        const std::optional<uint> m_offset;
        const bool m_checksum;
    };

    class SaveTotp: public AccountJob // clazy:exclude=ctor-missing-parent-argument
    {
        Q_OBJECT
    public:
        explicit SaveTotp(const SettingsProvider &settings,
                          const QUuid id, const QString &accountName, const QString &issuer,
                          const secrets::EncryptedSecret &secret, uint tokenLength,
                          uint timeStep, const QDateTime &epoch, Account::Hash hash,
                          const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);
        void run(void) override;
    Q_SIGNALS:
        void invalid(void);
        void saved(const QUuid id, const QString accountName, const QString issuer,
                   const QByteArray secret, const QByteArray nonce, uint tokenLength,
                   uint timeStep, const QDateTime epoch, accounts::Account::Hash hash);
    private:
        const SettingsProvider m_settings;
        const QUuid m_id;
        const QString m_accountName;
        const QString m_issuer;
        const secrets::EncryptedSecret m_secret;
        const uint m_tokenLength;
        const uint m_timeStep;
        const QDateTime m_epoch;
        const Account::Hash m_hash;
        const std::function<qint64(void)> m_clock;
    };

    class ComputeTotp: public AccountJob // clazy:exclude=ctor-missing-parent-argument
    {
        Q_OBJECT
    public:
        explicit ComputeTotp(const AccountSecret *secret,
                             const secrets::EncryptedSecret &tokenSecret, uint tokenLength,
                             const QDateTime &epoch, uint timeStep, const Account::Hash hash,
                             const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);
        void run(void) override;
    Q_SIGNALS:
        void otp(const QString otp, const QString nextOtp, const QDateTime validFrom, const QDateTime validUntil);
    private:
        const AccountSecret * const m_secret;
        const secrets::EncryptedSecret m_tokenSecret;
        const uint m_tokenLength;
        const QDateTime m_epoch;
        const uint m_timeStep;
        const Account::Hash m_hash;
        const std::function<qint64(void)> m_clock;
    };

    class ComputeHotp: public AccountJob // clazy:exclude=ctor-missing-parent-argument
    {
        Q_OBJECT
    public:
        explicit ComputeHotp(const AccountSecret *secret,
                             const secrets::EncryptedSecret &tokenSecret, uint tokenLength,
                             quint64 counter, const std::optional<uint> offset, bool checksum);
        void run(void) override;
    Q_SIGNALS:
        void otp(const QString otp, const QString nextOtp, const quint64 nextCounter);;
    private:
        const AccountSecret * const m_secret;
        const secrets::EncryptedSecret m_tokenSecret;
        const uint m_tokenLength;
        const quint64 m_counter;
        const std::optional<uint> m_offset;
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
