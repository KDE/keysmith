/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account_p.h"
#include "validation.h"

#include <QObject>
#include <QTimer>

namespace accounts
{
    QUuid AccountPrivate::id(void) const
    {
        return m_id;
    }

    int AccountPrivate::offset(void) const
    {
        return m_offset;
    }

    QString AccountPrivate::name(void) const
    {
        return m_name;
    }

    QString AccountPrivate::token(void) const
    {
        return m_token;
    }

    quint64 AccountPrivate::counter(void) const
    {
        return m_counter;
    }

    QDateTime AccountPrivate::epoch(void) const
    {
        return m_epoch;
    }

    uint AccountPrivate::timeStep(void) const
    {
        return m_timeStep;
    }

    Account::Hash AccountPrivate::hash(void) const
    {
        return m_hash;
    }

    Account::Algorithm AccountPrivate::algorithm(void) const
    {
        return m_algorithm;
    }

    int AccountPrivate::tokenLength(void) const
    {
        return m_tokenLength;
    }

    bool AccountPrivate::checksum(void) const
    {
        return m_checksum;
    }

    void AccountPrivate::setCounter(quint64 counter)
    {
        Q_Q(Account);
        if (m_is_still_alive && m_storage->isStillOpen() && m_algorithm == Account::Algorithm::Hotp) {
            SaveHotp *job = new SaveHotp(m_storage->settings(),m_id, m_name, m_secret, counter, m_tokenLength);
            m_actions->queueAndProceed(job, [counter, job, q, this](void) -> void
            {
                new HandleCounterUpdate(this, counter, job, q);
            });
        }
        // TODO: warn if not
    }

    void AccountPrivate::acceptCounter(quint64 counter)
    {
        if (m_is_still_alive && m_storage->isStillOpen() && m_algorithm == Account::Algorithm::Hotp) {
            m_counter = counter;
            recompute();
        }
        // TODO: warn if not
    }

    void AccountPrivate::acceptToken(QString token)
    {
        Q_Q(Account);
        if (m_is_still_alive && m_storage->isStillOpen()) {
            m_token = token;
            Q_EMIT q->tokenChanged(m_token);
        }
        // TODO: warn if not
    }

    void AccountPrivate::recompute(void)
    {
        Q_Q(Account);
        if (m_is_still_alive && m_storage->isStillOpen()) {
            ComputeHotp *hotpJob = nullptr;
            ComputeTotp *totpJob = nullptr;

            switch (m_algorithm) {
            case Account::Algorithm::Hotp:
                hotpJob = new ComputeHotp(m_secret, m_counter, m_tokenLength, m_offset, m_checksum);
                m_actions->queueAndProceed(hotpJob, [hotpJob, q, this](void) -> void {
                    new HandleTokenUpdate(this, hotpJob, q);
                });
                break;
            case Account::Algorithm::Totp:
                totpJob = new ComputeTotp(m_secret, m_epoch, m_timeStep, m_tokenLength, m_hash);
                m_actions->queueAndProceed(totpJob, [totpJob, q, this](void) -> void
                {
                    new HandleTokenUpdate(this, totpJob, q);
                });
                break;
            default:
                break;
            }
        }
        // TODO: warn if not
    }

    void AccountPrivate::remove(void)
    {
        if (m_is_still_alive && m_storage->isStillOpen()) {
            QSet<QString> self;
            self.insert(m_name);
            m_storage->removeAccounts(self);
        }
        // TODO: warn if not
    }

    void AccountPrivate::markForRemoval(void)
    {
        m_is_still_alive = false;
    }

    bool AccountPrivate::isStillAlive(void) const
    {
        return m_is_still_alive;
    }

    AccountPrivate::AccountPrivate(const std::function<Account*(AccountPrivate*)> &account, AccountStoragePrivate *storage, Dispatcher *dispatcher, const QUuid &id,
                                   const QString &name, const QString &secret, quint64 counter, int tokenLength, int offset, bool addChecksum) :
        q_ptr(account(this)), m_storage(storage), m_actions(dispatcher), m_is_still_alive(true), m_algorithm(Account::Algorithm::Hotp), m_id(id), m_token(QString()),
        m_name(name), m_secret(secret), m_tokenLength(tokenLength),
        m_counter(counter), m_offset(offset), m_checksum(addChecksum),
        m_epoch(QDateTime::fromMSecsSinceEpoch(0)), m_timeStep(30), m_hash(Account::Hash::Default) // not a totp token so these values don't really matter
    {
    }

    AccountPrivate::AccountPrivate(const std::function<Account*(AccountPrivate*)> &account, AccountStoragePrivate *storage, Dispatcher *dispatcher, const QUuid &id,
                                   const QString &name, const QString &secret, const QDateTime &epoch, uint timeStep, int tokenLength, Account::Hash hash) :
        q_ptr(account(this)), m_storage(storage), m_actions(dispatcher), m_is_still_alive(true), m_algorithm(Account::Algorithm::Totp), m_id(id), m_token(QString()),
        m_name(name), m_secret(secret), m_tokenLength(tokenLength),
        m_counter(0), m_offset(-1), m_checksum(false), // not a hotp token so these values don't really matter
        m_epoch(epoch), m_timeStep(timeStep), m_hash(hash)
    {
    }

    QVector<QString> AccountStoragePrivate::activeAccounts(void) const
    {
        QVector<QString> active;

        if (m_is_still_open) {
            const QList<QString> all = m_names.keys();
            for (const QString &account : all) {
                if (m_accountsPrivate[m_names[account]]->isStillAlive()) {
                    active.append(account);
                }
                // TODO: log if not
            }
        }

        return active;
    }

    bool AccountStoragePrivate::isStillOpen(void) const
    {
        return m_is_still_open;
    }

    bool AccountStoragePrivate::isNameStillAvailable(const QString &name) const
    {
        return m_is_still_open && !m_names.contains(name);
    }

    bool AccountStoragePrivate::contains(const QString &account) const
    {
        /*
         * Pretend an account which is marked for removal is already fully purged
         * This lets the behaviour of get() and contains() be mutually consistent
         * without having to hand out pointers which may be about to go stale in get()
         */
        return m_is_still_open && m_names.contains(account) && m_accountsPrivate[m_names[account]]->isStillAlive();
        // TODO: warn if not still open, log if account not alive
    }

    Account * AccountStoragePrivate::get(const QString &account) const
    {
        // TODO: warn if not
        return contains(account) ? m_accounts[m_names[account]] : nullptr;
    }

    SettingsProvider AccountStoragePrivate::settings(void) const
    {
        return m_settings;
    }

    void AccountStoragePrivate::removeAccounts(const QSet<QString> &accountNames)
    {
        if (m_is_still_open) {
            QSet<QUuid> ids;
            for (const QString &accountName : accountNames) {
                if (m_names.contains(accountName)) {
                    const QUuid id = m_names[accountName];
                    AccountPrivate *p = m_accountsPrivate[id];
                    /*
                     * Avoid doing anything with accounts which are already about to be removed from the maps
                     */
                    if (p->isStillAlive()) {
                        p->markForRemoval();
                        ids.insert(id);
                    }
                    // TODO: log if not
                }
            }

            DeleteAccounts *job = new DeleteAccounts(m_settings, ids);
            m_actions->queueAndProceed(job, [&ids, job, this](void) -> void
            {
                for (const QUuid &id : ids) {
                    Account *account = m_accounts[id];
                    QObject::connect(job, &DeleteAccounts::finished, account, &Account::removed);
                }
            });
        }
        // TODO: warn if not
    }

    void AccountStoragePrivate::acceptAccountRemoval(const QString &accountName)
    {
        if (m_names.contains(accountName)) {
            const QUuid id = m_names[accountName];
            Account *account = m_accounts[id];
            m_accounts.remove(id);
            m_accountsPrivate.remove(id);
            m_names.remove(accountName);
            m_ids.remove(id);
            QTimer::singleShot(0, account, &accounts::Account::deleteLater);
        }
        // TODO: warn if not
    }

    void AccountStoragePrivate::dispose(const std::function<void(Null*)> &handler)
    {
        if (m_is_still_open) {
            m_is_still_open = false;
            Null *job = new Null();
            m_actions->queueAndProceed(job, [job, &handler](void) -> void
            {
                handler(job);
            });
        }
        // TODO: warn if not
    }

    void AccountStoragePrivate::acceptDisposal(void)
    {
        Q_Q(AccountStorage);
        if (!m_is_still_open) {
            for (const QString &accountName : m_names.keys()) {
                const QUuid id = m_names[accountName];
                Account *account = m_accounts[id];
                m_accounts.remove(id);
                m_accountsPrivate.remove(id);
                m_names.remove(accountName);
                m_ids.remove(id);
                QTimer::singleShot(0, account, &accounts::Account::deleteLater);
            }
            Q_EMIT q->disposed();
        }
        // TODO: warn if not
    }

    QUuid AccountStoragePrivate::generateId(const QString &name) const
    {
        QUuid attempt = QUuid::createUuidV5(QUuid(), name);
        while (attempt.isNull() || m_ids.contains(attempt)) {
            attempt = QUuid::createUuid();
        }
        return attempt;
    }

    void AccountStoragePrivate::addHotp(const std::function<void(SaveHotp*)> &handler, const QString &name, const QString &secret, quint64 counter, int tokenLength, int offset, bool addChecksum)
    {
        Q_UNUSED(offset);
        Q_UNUSED(addChecksum);
        if (m_is_still_open && isNameStillAvailable(name)) {
            QUuid id = generateId(name);
            if (checkHotpAccount(id, name, secret, tokenLength)) {
                m_ids.insert(id);
                SaveHotp *job = new SaveHotp(m_settings, id, name, secret, counter, tokenLength);
                m_actions->queueAndProceed(job, [job, &handler](void) -> void
                {
                    handler(job);
                });
            }
        }
        // TODO: warn if not
    }

    void AccountStoragePrivate::addTotp(const std::function<void(SaveTotp*)> &handler, const QString &name, const QString &secret, uint timeStep, int tokenLength, const QDateTime &epoch, Account::Hash hash)
    {
        Q_UNUSED(epoch);
        Q_UNUSED(hash);
        if (m_is_still_open && isNameStillAvailable(name)) {
            QUuid id = generateId(name);
            if (checkTotpAccount(id, name, secret, tokenLength, timeStep)) {
                m_ids.insert(id);
                SaveTotp *job = new SaveTotp(m_settings, id, name, secret, timeStep, tokenLength);
                m_actions->queueAndProceed(job, [job, &handler](void) -> void
                {
                    handler(job);
                });
            }
        }
        // TODO: warn if not
    }

    void AccountStoragePrivate::load(const std::function<void(LoadAccounts*)> &handler)
    {
        if (m_is_still_open) {
            LoadAccounts *job = new LoadAccounts(m_settings);
            m_actions->queueAndProceed(job, [job, &handler](void) -> void
            {
                handler(job);
            });
        }
        // TODO: warn if not
    }

    Account * AccountStoragePrivate::acceptHotpAccount(const QUuid &id, const QString &name, const QString &secret, quint64 counter, int tokenLength, int offset, bool addChecksum)
    {
        Q_Q(AccountStorage);
        const std::function<Account*(AccountPrivate*)> registration([this, q, &id](AccountPrivate *p) -> Account *
        {
            Account *account = new Account(p, q);
            m_accounts.insert(id, account);
            return account;
        });
        m_ids.insert(id);
        m_names.insert(name, id);
        m_accountsPrivate.insert(id, new AccountPrivate(registration, this, m_actions, id, name, secret, counter, tokenLength, offset, addChecksum));

        Q_ASSERT_X(m_accounts.contains(id), Q_FUNC_INFO, "account should have been registered");
        return m_accounts[id];
    }

    Account * AccountStoragePrivate::acceptTotpAccount(const QUuid &id, const QString &name, const QString &secret, uint timeStep, int tokenLength, const QDateTime &epoch, Account::Hash hash)
    {
        Q_Q(AccountStorage);
        const std::function<Account*(AccountPrivate*)> registration([this, q, &id](AccountPrivate *p) -> Account *
        {
            Account *account = new Account(p, q);
            m_accounts.insert(id, account);
            return account;
        });
        m_ids.insert(id);
        m_names.insert(name, id);
        m_accountsPrivate.insert(id, new AccountPrivate(registration, this, m_actions, id, name, secret, epoch, timeStep, tokenLength, hash));

        Q_ASSERT_X(m_accounts.contains(id), Q_FUNC_INFO, "account should have been registered");
        return m_accounts[id];
    }

    AccountStoragePrivate::AccountStoragePrivate(const SettingsProvider &settings, AccountStorage *storage, Dispatcher *dispatcher) :
        q_ptr(storage), m_is_still_open(true), m_actions(dispatcher), m_settings(settings)
    {
    }

    HandleCounterUpdate::HandleCounterUpdate(AccountPrivate *account, quint64 counter, SaveHotp *job, QObject *parent) :
        QObject(parent), m_accept_on_finish(true), m_counter(counter), m_account(account)
    {
        QObject::connect(job, &SaveHotp::invalid, this, &HandleCounterUpdate::rejected);
        QObject::connect(job, &SaveHotp::finished, this, &HandleCounterUpdate::finished);
    }

    void HandleCounterUpdate::rejected(void)
    {
        m_accept_on_finish = false;
    }

    void HandleCounterUpdate::finished(void)
    {
        if (m_accept_on_finish) {
            m_account->acceptCounter(m_counter);
        }
        // TODO: warn if not
        deleteLater();
    }

    HandleTokenUpdate::HandleTokenUpdate(AccountPrivate *account, ComputeHotp *job, QObject *parent) :
        QObject(parent), m_account(account)
    {
        QObject::connect(job, &ComputeHotp::otp, this, &HandleTokenUpdate::token);
        QObject::connect(job, &ComputeHotp::finished, this, &HandleTokenUpdate::deleteLater);
    }

    HandleTokenUpdate::HandleTokenUpdate(AccountPrivate *account, ComputeTotp *job, QObject *parent) :
        QObject(parent), m_account(account)
    {
        QObject::connect(job, &ComputeTotp::otp, this, &HandleTokenUpdate::token);
        QObject::connect(job, &ComputeTotp::finished, this, &HandleTokenUpdate::deleteLater);
    }

    void HandleTokenUpdate::token(QString otp)
    {
        m_account->acceptToken(otp);
    }
}
