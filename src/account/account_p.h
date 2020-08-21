/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_ACCOUNT_PRIVATE_H
#define ACCOUNTS_ACCOUNT_PRIVATE_H

#include "account.h"
#include "actions_p.h"
#include "keys.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>

#include <functional>
#include <optional>

namespace accounts
{
    class AccountStoragePrivate;
    class AccountPrivate
    {
    public:
        QUuid id(void) const;
        std::optional<uint> offset(void) const;
        QString name(void) const;
        QString token(void) const;
        QString issuer(void) const;
        quint64 counter(void) const;
        QDateTime epoch(void) const;
        uint timeStep(void) const;
        Account::Hash hash(void) const;
        Account::Algorithm algorithm(void) const;
        int tokenLength(void) const;
        bool checksum(void) const;
    public:
        static QString toFullName(const QString &name, const QString &issuer);
        explicit AccountPrivate(const std::function<Account*(AccountPrivate*)> &account,
                                AccountStoragePrivate *storage, Dispatcher *dispatcher,
                                const QUuid &id, const QString &name, const QString &issuer,
                                const secrets::EncryptedSecret &secret, uint tokenLength,
                                quint64 counter, const std::optional<uint> &offset, bool addChecksum);
        explicit AccountPrivate(const std::function<Account*(AccountPrivate*)> &account,
                                AccountStoragePrivate *storage, Dispatcher *dispatcher,
                                const QUuid &id, const QString &name, const QString &issuer,
                                const secrets::EncryptedSecret &secret, uint tokenLength,
                                const QDateTime &epoch, uint timeStep, Account::Hash hash);
        void recompute(void);
        void setCounter(quint64 counter);
        void remove(void);
        void acceptCounter(quint64 counter);
        void acceptTotpTokens(QString token, QString nextToken, QDateTime validFrom, QDateTime validUntil);
        void acceptHotpTokens(QString token, QString nextToken, quint64 validUntil);
        void markForRemoval(void);
        bool isStillAlive(void) const;
    private:
        void setToken(QString token);
        void shiftTokens(void);
    private:
        Q_DISABLE_COPY(AccountPrivate);
        Q_DECLARE_PUBLIC(Account);
        Account * const q_ptr;
    private:
        AccountStoragePrivate *m_storage;
        Dispatcher * m_actions;
        bool m_is_still_alive;
        const Account::Algorithm m_algorithm;
        const QUuid m_id;
    private:
        QString m_token;
        QString m_nextToken;
        QDateTime m_nextTotpValidFrom;
        QDateTime m_nextTotpValidUntil;
        quint64 m_nextCounter;
    private:
        const QString m_name;
        const QString m_issuer;
        const secrets::EncryptedSecret m_secret;
        const uint m_tokenLength;
        quint64 m_counter;
        const std::optional<uint> m_offset;
        const bool m_checksum;
        const QDateTime m_epoch;
        const qint64 m_timeStep;
        const Account::Hash m_hash;
    };

    class AccountStoragePrivate
    {
    public:
        explicit AccountStoragePrivate(const SettingsProvider &settings,
                                       AccountSecret *secret, AccountStorage *storage, Dispatcher *dispatcher);
        void dispose(const std::function<void(Null*)> &handler);
        void acceptDisposal(void);
        void unlock(const std::function<void(RequestAccountPassword*)> &handler);
        void load(const std::function<void(LoadAccounts*)> &handler);
        QVector<QString> activeAccounts(void) const;
        bool isStillOpen(void) const;
        bool contains(const QString &account) const;
        SettingsProvider settings(void) const;
        bool isAccountStillAvailable(const QString &account) const;
        Account * get(const QString &account) const;
        AccountSecret *secret(void) const;
        void removeAccounts(const QSet<QString> &accountNames);
        void acceptAccountRemoval(const QString &accountName);
        Account * acceptHotpAccount(const QUuid &id, const QString &name,  const QString &issuer,
                                    const secrets::EncryptedSecret &secret, uint tokenLength,
                                    quint64 counter, const std::optional<uint> &offset, bool checksum);
        Account * acceptTotpAccount(const QUuid &id, const QString &name, const QString &issuer,
                                    const secrets::EncryptedSecret &secret, uint tokenLength,
                                    uint timeStep, const QDateTime &epoch, Account::Hash hash);
        bool addHotp(const std::function<void(SaveHotp*)> &handler,
                     const QString &name, const QString &issuer,
                     const QString &secret, uint tokenLength,
                     quint64 counter, const std::optional<uint> &offset, bool checksum);
        bool addTotp(const std::function<void(SaveTotp*)> &handler,
                     const QString &name, const QString &issuer,
                     const QString &secret, uint tokenLength,
                     uint timeStep, const QDateTime &epoch, Account::Hash hash);
        void notifyLoaded(void);
        bool isLoaded(void) const;
        void notifyError(void);
        void clearError(void);
        bool hasError(void) const;
    private:
        bool validateGenericNewToken(const QString &name, const QString &issuer,
                                     const QString &secret, uint tokenLength) const;
        std::optional<secrets::EncryptedSecret> encrypt(const QString &secret) const;
        QUuid generateId(const QString &name) const;
    private:
        Q_DECLARE_PUBLIC(AccountStorage);
        Q_DISABLE_COPY(AccountStoragePrivate);
        AccountStorage * const q_ptr;
    private:
        bool m_is_loaded;
        bool m_has_error;
        bool m_is_still_open;
        Dispatcher * const m_actions;
        const SettingsProvider m_settings;
        AccountSecret * m_secret;
    private:
        QSet<QUuid> m_ids;
        QHash<QString, QUuid> m_names;
        QHash<QUuid, Account*> m_accounts;
        QHash<QUuid, AccountPrivate*> m_accountsPrivate;
    };

    class HandleCounterUpdate: public QObject
    {
        Q_OBJECT
    public:
        explicit HandleCounterUpdate(AccountPrivate *account, AccountStoragePrivate *storage,
                                     quint64 counter, SaveHotp *job, QObject *parent = nullptr);
    private:
        bool m_accept_on_finish;
        const quint64 m_counter;
        AccountPrivate * const m_account;
        AccountStoragePrivate * const m_storage;
    private Q_SLOTS:
        void rejected(void);
        void finished(void);
    };

    class HandleTokenUpdate: public QObject
    {
        Q_OBJECT
    public:
        explicit HandleTokenUpdate(AccountPrivate *account, ComputeHotp *job, QObject *parent = nullptr);
        explicit HandleTokenUpdate(AccountPrivate *account, ComputeTotp *job, QObject *parent = nullptr);
    private:
        AccountPrivate * const m_account;
    private Q_SLOTS:
        void totp(QString otp, QString nextOtp, QDateTime validFrom, QDateTime validUntil);
        void hotp(QString otp, QString nextOtp, quint64 validUntil);
    };
}

#endif
