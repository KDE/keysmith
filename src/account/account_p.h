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

namespace accounts
{
    class AccountStoragePrivate;
    class AccountPrivate
    {
    public:
        QUuid id(void) const;
        int offset(void) const;
        QString name(void) const;
        QString token(void) const;
        quint64 counter(void) const;
        QDateTime epoch(void) const;
        uint timeStep(void) const;
        Account::Hash hash(void) const;
        Account::Algorithm algorithm(void) const;
        int tokenLength(void) const;
        bool checksum(void) const;
    public:
        explicit AccountPrivate(const std::function<Account*(AccountPrivate*)> &account,
                                AccountStoragePrivate *storage, Dispatcher *dispatcher, const QUuid &id,
                                const QString &name, const QString &secret,
                                quint64 counter, int tokenLength, int offset, bool addChecksum);
        explicit AccountPrivate(const std::function<Account*(AccountPrivate*)> &account,
                                AccountStoragePrivate *storage, Dispatcher *dispatcher, const QUuid &id,
                                const QString &name, const QString &secret,
                                const QDateTime &epoch, uint timeStep, int tokenLength, Account::Hash hash);
        void recompute(void);
        void setCounter(quint64 counter);
        void remove(void);
        void acceptCounter(quint64 counter);
        void acceptToken(QString token);
        void markForRemoval(void);
        bool isStillAlive(void) const;
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
        const QString m_name;
        const QString m_secret;
        const int m_tokenLength;
        quint64 m_counter;
        const int m_offset;
        const bool m_checksum;
        const QDateTime m_epoch;
        const qint64 m_timeStep;
        const Account::Hash m_hash;
    };

    class AccountStoragePrivate
    {
    public:
        explicit AccountStoragePrivate(const SettingsProvider &settings, AccountSecret *secret, AccountStorage *storage, Dispatcher *dispatcher);
        void dispose(const std::function<void(Null*)> &handler);
        void acceptDisposal(void);
        void unlock(const std::function<void(RequestAccountPassword*)> &handler);
        void load(const std::function<void(LoadAccounts*)> &handler);
        QVector<QString> activeAccounts(void) const;
        bool isStillOpen(void) const;
        bool contains(const QString &account) const;
        SettingsProvider settings(void) const;
        bool isNameStillAvailable(const QString &name) const;
        Account * get(const QString &account) const;
        AccountSecret *secret(void) const;
        void removeAccounts(const QSet<QString> &accountNames);
        void acceptAccountRemoval(const QString &accountName);
        Account * acceptHotpAccount(const QUuid &id,
                                    const QString &name,
                                    const QString &secret,
                                    quint64 counter = 0ULL,
                                    int tokenLength = 6,
                                    int offset = -1,
                                    bool addChecksum = false);
        Account * acceptTotpAccount(const QUuid &id,
                                    const QString &name,
                                    const QString &secret,
                                    uint timeStep = 30,
                                    int tokenLength = 6,
                                    const QDateTime &epoch = QDateTime::fromMSecsSinceEpoch(0),
                                    Account::Hash hash = Account::Hash::Default);
        void addHotp(const std::function<void(SaveHotp*)> &handler,
                     const QString &name,
                     const QString &secret,
                     quint64 counter = 0ULL,
                     int tokenLength = 6,
                     int offset = -1,
                     bool addChecksum = false);
        void addTotp(const std::function<void(SaveTotp*)> &handler,
                     const QString &name,
                     const QString &secret,
                     uint timeStep = 30,
                     int tokenLength = 6,
                     const QDateTime &epoch = QDateTime::fromMSecsSinceEpoch(0),
                     Account::Hash hash = Account::Hash::Default);
    private:
        QUuid generateId(const QString &name) const;
    private:
        Q_DECLARE_PUBLIC(AccountStorage);
        Q_DISABLE_COPY(AccountStoragePrivate);
        AccountStorage * const q_ptr;
    private:
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
        explicit HandleCounterUpdate(AccountPrivate *account, quint64 counter, SaveHotp * job, QObject *parent = nullptr);
    private:
        bool m_accept_on_finish;
        const quint64 m_counter;
        AccountPrivate * const m_account;
    private Q_SLOTS:
        void rejected(void);
        void finished(void);
    };

    class HandleTokenUpdate: public QObject
    {
        Q_OBJECT
    public:
        explicit HandleTokenUpdate(AccountPrivate *account, ComputeHotp * job, QObject *parent = nullptr);
        explicit HandleTokenUpdate(AccountPrivate *account, ComputeTotp * job, QObject *parent = nullptr);
    private:
        AccountPrivate * const m_account;
    private Q_SLOTS:
        void token(QString otp);
    };
}

#endif
