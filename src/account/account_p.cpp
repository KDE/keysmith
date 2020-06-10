/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account_p.h"
#include "validation.h"

#include "../logging_p.h"

#include <QObject>
#include <QTimer>

KEYSMITH_LOGGER(logger, ".accounts.account_p")

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


    QString AccountPrivate::issuer(void) const
    {
        return m_issuer;
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
        if (!m_is_still_alive) {
            qCDebug(logger)
                << "Will not set counter for account:" << m_id
                << "Object marked for death";
            return;
        }

        if (!m_storage->isStillOpen()) {
            qCDebug(logger)
                << "Will not set counter for account:" << m_id
                << "Storage no longer open";
            return;
        }

        if (m_algorithm != Account::Algorithm::Hotp) {
            qCDebug(logger)
                << "Will not set counter for account:" << m_id
                << "Algorithm not applicable:" << m_algorithm;
            return;
        }

        qCDebug(logger) << "Requesting to store updated details for account:" << m_id;
        SaveHotp *job = new SaveHotp(m_storage->settings(),m_id, m_name, m_issuer, m_secret, counter, m_tokenLength);
        m_actions->queueAndProceed(job, [counter, job, q, this](void) -> void
        {
            new HandleCounterUpdate(this, m_storage, counter, job, q);
        });
    }

    void AccountPrivate::acceptCounter(quint64 counter)
    {
        if (!m_is_still_alive) {
            qCDebug(logger)
                << "Ignoring counter update for account:" << m_id
                << "Object marked for death";
            return;
        }

        if (!m_storage->isStillOpen()) {
            qCDebug(logger)
                << "Ignoring counter update for account:" << m_id
                << "Storage no longer open";
            return;
        }

        if (m_algorithm != Account::Algorithm::Hotp) {
            qCDebug(logger)
                << "Ignoring counter update for account:" << m_id
                << "Algorithm not applicable:" << m_algorithm;
            return;
        }

        qCDebug(logger) << "Counter is updated for account:" << m_id;
        m_counter = counter;
        recompute();
    }

    void AccountPrivate::acceptToken(QString token)
    {
        Q_Q(Account);
        if (!m_is_still_alive) {
            qCDebug(logger)
                << "Ignoring token update for account:" << m_id
                << "Object marked for death";
            return;
        }

        if (!m_storage->isStillOpen()) {
            qCDebug(logger)
                << "Ignoring token update for account:" << m_id
                << "Storage no longer open";
            return;
        }

        qCDebug(logger) << "Token is updated for account:" << m_id;
        m_token = token;
        Q_EMIT q->tokenChanged(m_token);
    }

    void AccountPrivate::recompute(void)
    {
        Q_Q(Account);
        if (!m_is_still_alive) {
            qCDebug(logger)
                << "Will not recompute token for account:" << m_id
                << "Object marked for death";
            return;
        }

        if (!m_storage->isStillOpen()) {
            qCDebug(logger)
                << "Will not recompute token for account:" << m_id
                << "Storage no longer open";
            return;
        }

        qCDebug(logger) << "Requesting recomputed token for account:" << m_id;
        ComputeHotp *hotpJob = nullptr;
        ComputeTotp *totpJob = nullptr;

        switch (m_algorithm) {
        case Account::Algorithm::Hotp:
            hotpJob = new ComputeHotp(m_storage->secret(), m_secret, m_counter, m_tokenLength, m_offset, m_checksum);
            m_actions->queueAndProceed(hotpJob, [hotpJob, q, this](void) -> void {
                new HandleTokenUpdate(this, hotpJob, q);
            });
            break;
        case Account::Algorithm::Totp:
            totpJob = new ComputeTotp(m_storage->secret(), m_secret, m_epoch, m_timeStep, m_tokenLength, m_hash);
            m_actions->queueAndProceed(totpJob, [totpJob, q, this](void) -> void
            {
                new HandleTokenUpdate(this, totpJob, q);
            });
            break;
        default:
            Q_ASSERT_X(false, Q_FUNC_INFO, "unknown algorithm value");
            break;
        }
    }

    void AccountPrivate::remove(void)
    {
        if (!m_is_still_alive) {
            qCDebug(logger)
                << "Will not remove account:" << m_id
                << "Object marked for death";
            return;
        }

        if (!m_storage->isStillOpen()) {
            qCDebug(logger)
                << "Will not remove account:" << m_id
                << "Storage no longer open";
            return;
        }

        QSet<QString> self;
        self.insert(AccountPrivate::toFullName(m_name, m_issuer));
        m_storage->removeAccounts(self);
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
                                   const QString &name, const QString &issuer, const secrets::EncryptedSecret &secret, quint64 counter, int tokenLength, int offset, bool addChecksum) :
        q_ptr(account(this)), m_storage(storage), m_actions(dispatcher), m_is_still_alive(true), m_algorithm(Account::Algorithm::Hotp), m_id(id), m_token(QString()),
        m_name(name), m_issuer(issuer), m_secret(secret), m_tokenLength(tokenLength),
        m_counter(counter), m_offset(offset), m_checksum(addChecksum),
        m_epoch(QDateTime::fromMSecsSinceEpoch(0)), m_timeStep(30), m_hash(Account::Hash::Default) // not a totp token so these values don't really matter
    {
    }

    AccountPrivate::AccountPrivate(const std::function<Account*(AccountPrivate*)> &account, AccountStoragePrivate *storage, Dispatcher *dispatcher, const QUuid &id,
                                   const QString &name, const QString &issuer, const secrets::EncryptedSecret &secret, const QDateTime &epoch, uint timeStep, int tokenLength, Account::Hash hash) :
        q_ptr(account(this)), m_storage(storage), m_actions(dispatcher), m_is_still_alive(true), m_algorithm(Account::Algorithm::Totp), m_id(id), m_token(QString()),
        m_name(name), m_issuer(issuer), m_secret(secret), m_tokenLength(tokenLength),
        m_counter(0), m_offset(-1), m_checksum(false), // not a hotp token so these values don't really matter
        m_epoch(epoch), m_timeStep(timeStep), m_hash(hash)
    {
    }

    QVector<QString> AccountStoragePrivate::activeAccounts(void) const
    {
        QVector<QString> active;
        if (!m_is_still_open) {
            qCDebug(logger) << "Not returning accounts: account storage no longer open";
            return active;
        }

        const QList<QString> all = m_names.keys();
        for (const QString &account : all) {
            const QUuid id = m_names[account];
            if (m_accountsPrivate[id]->isStillAlive()) {
                active.append(account);
            } else {
                qCDebug(logger)
                    << "Not returning account:" << id
                    << "Object marked for death";
            }
        }

        return active;
    }

    bool AccountStoragePrivate::isStillOpen(void) const
    {
        return m_is_still_open;
    }

    bool AccountStoragePrivate::isAccountStillAvailable(const QString &account) const
    {
        if (!m_is_still_open) {
            qCDebug(logger) << "Pretending no name is available: account storage no longer open";
            return false;
        }

        return !m_names.contains(account);
    }

    QString AccountPrivate::toFullName(const QString &name, const QString &issuer)
    {
        return issuer.isEmpty() ? name : issuer + QLatin1Char(':') + name;
    }

    bool AccountStoragePrivate::contains(const QString &account) const
    {
        /*
         * Pretend an account which is marked for removal is already fully purged
         * This lets the behaviour of get() and contains() be mutually consistent
         * without having to hand out pointers which may be about to go stale in get()
         */
        if (!m_is_still_open) {
            qCDebug(logger) << "Pretending no account exists: account storage no longer open";
            return false;
        }

        if (!m_names.contains(account)) {
            return false;
        }

        const QUuid id = m_names[account];
        if (!m_accountsPrivate[id]->isStillAlive()) {
            qCDebug(logger)
                << "Pretending account does not exist:" << id
                << "Object marked for death";
            return false;
        }

        return true;
    }

    Account * AccountStoragePrivate::get(const QString &account) const
    {
        return contains(account) ? m_accounts[m_names[account]] : nullptr;
    }

    SettingsProvider AccountStoragePrivate::settings(void) const
    {
        return m_settings;
    }

    AccountSecret * AccountStoragePrivate::secret(void) const
    {
        return m_secret;
    }

    void AccountStoragePrivate::removeAccounts(const QSet<QString> &accountNames)
    {
        if (!m_is_still_open) {
            qCDebug(logger) << "Not removing accounts: account storage no longer open";
            return;
        }

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
                } else {
                    qCDebug(logger)
                        << "Not removing account:" << id
                        << "Object marked for death";
                }
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

    void AccountStoragePrivate::acceptAccountRemoval(const QString &accountName)
    {
        if (!m_names.contains(accountName)) {
            qCDebug(logger) << "Not accepting account removal: account name unknown";
            return;
        }

        const QUuid id = m_names[accountName];
        qCDebug(logger) << "Handling account cleanup for account:" << id;

        Account *account = m_accounts[id];
        m_accounts.remove(id);
        m_accountsPrivate.remove(id);
        m_names.remove(accountName);
        m_ids.remove(id);
        QTimer::singleShot(0, account, &accounts::Account::deleteLater);
    }

    void AccountStoragePrivate::dispose(const std::function<void(Null*)> &handler)
    {
        if (!m_is_still_open) {
            qCDebug(logger) << "Not disposing of storage: account storage no longer open";
            return;
        }

        qCDebug(logger) << "Disposing of storage";

        m_is_still_open = false;
        Null *job = new Null();
        m_secret->cancelRequests();
        m_actions->queueAndProceed(job, [job, &handler](void) -> void
        {
            handler(job);
        });
    }

    void AccountStoragePrivate::acceptDisposal(void)
    {
        Q_Q(AccountStorage);
        if (m_is_still_open) {
            qCDebug(logger) << "Ignoring disposal of storage: account storage is still open";
            return;
        }

        qCDebug(logger) << "Handling storage disposal";

        for (const QString &accountName : m_names.keys()) {
            const QUuid id = m_names[accountName];
            qCDebug(logger) << "Handling account cleanup for account:" << id;

            Account *account = m_accounts[id];
            m_accounts.remove(id);
            m_accountsPrivate.remove(id);
            m_names.remove(accountName);
            m_ids.remove(id);
            QTimer::singleShot(0, account, &accounts::Account::deleteLater);
        }
        QTimer::singleShot(0, m_secret, &accounts::AccountSecret::deleteLater);
        Q_EMIT q->disposed();
    }

    QUuid AccountStoragePrivate::generateId(const QString &name) const
    {
        QUuid attempt = QUuid::createUuidV5(QUuid(), name);
        while (attempt.isNull() || m_ids.contains(attempt)) {
            attempt = QUuid::createUuid();
        }
        return attempt;
    }

    std::optional<secrets::EncryptedSecret> AccountStoragePrivate::encrypt(const QString &secret) const
    {
        if (!m_is_still_open) {
            qCDebug(logger) << "Will not encrypt account secret: storage no longer open";
            return std::nullopt;
        }

        if (!m_secret || !m_secret->key()) {
            qCDebug(logger) << "Will not encrypt account secret: encryption key not available";
            return std::nullopt;
        }

        QScopedPointer<secrets::SecureMemory> decoded(secrets::decodeBase32(secret));
        if (!decoded) {
            qCDebug(logger) << "Will not encrypt account secret: failed to decode base32";
            return std::nullopt;
        }

        return m_secret->encrypt(decoded.data());
    }

    bool AccountStoragePrivate::validateGenericNewToken(const QString &name, const QString &issuer, const QString &secret, int tokenLength) const
    {
        return checkTokenLength(tokenLength) && checkName(name) && checkIssuer(issuer) && isAccountStillAvailable(AccountPrivate::toFullName(name, issuer)) && checkSecret(secret);
    }

    bool AccountStoragePrivate::addHotp(const std::function<void(SaveHotp*)> &handler, const QString &name, const QString &issuer, const QString &secret, quint64 counter, int tokenLength, int offset, bool addChecksum)
    {
        Q_UNUSED(offset);
        Q_UNUSED(addChecksum);
        if (!m_is_still_open) {
            qCDebug(logger) << "Will not add new HOTP account: storage no longer open";
            return false;
        }

        if (!validateGenericNewToken(name, issuer, secret, tokenLength)) {
            qCDebug(logger) << "Will not add new HOTP account: invalid account details";
            return false;
        }

        std::optional<secrets::EncryptedSecret> encryptedSecret = encrypt(secret);
        if (!encryptedSecret) {
            qCDebug(logger) << "Will not add new HOTP account: failed to encrypt secret";
            return false;
        }

        QUuid id = generateId(AccountPrivate::toFullName(name, issuer));
        qCDebug(logger) << "Requesting to store details for new HOTP account:" << id;

        m_ids.insert(id);
        SaveHotp *job = new SaveHotp(m_settings, id, name, issuer, *encryptedSecret, counter, tokenLength);
        m_actions->queueAndProceed(job, [job, &handler](void) -> void
        {
            handler(job);
        });
        return true;
    }

    bool AccountStoragePrivate::addTotp(const std::function<void(SaveTotp*)> &handler, const QString &name, const QString &issuer, const QString &secret, uint timeStep, int tokenLength, const QDateTime &epoch, Account::Hash hash)
    {
        Q_UNUSED(epoch);
        Q_UNUSED(hash);
        if (!m_is_still_open) {
            qCDebug(logger) << "Will not add new TOTP account: storage no longer open";
            return false;
        }

        if (!validateGenericNewToken(name, issuer, secret, tokenLength) || !checkTimeStep(timeStep)) {
            qCDebug(logger) << "Will not add new TOTP account: invalid account details";
            return false;
        }

        std::optional<secrets::EncryptedSecret> encryptedSecret = encrypt(secret);
        if (!encryptedSecret) {
            qCDebug(logger) << "Will not add new TOTP account: failed to encrypt secret";
            return false;
        }

        QUuid id = generateId(AccountPrivate::toFullName(name, issuer));
        qCDebug(logger) << "Requesting to store details for new TOTP account:" << id;

        m_ids.insert(id);
        SaveTotp *job = new SaveTotp(m_settings, id, name, issuer, *encryptedSecret, timeStep, tokenLength);
        m_actions->queueAndProceed(job, [job, &handler](void) -> void
        {
            handler(job);
        });
        return true;
    }

    void AccountStoragePrivate::unlock(const std::function<void(RequestAccountPassword*)> &handler)
    {
        if (!m_is_still_open) {
            qCDebug(logger) << "Will not attempt to unlock accounts: storage no longer open";
            return;
        }

        qCDebug(logger) << "Requesting to unlock account storage";
        RequestAccountPassword *job = new RequestAccountPassword(m_settings, m_secret);
        m_actions->queueAndProceed(job, [job, &handler](void) -> void
        {
            handler(job);
        });
    }

    void AccountStoragePrivate::load(const std::function<void(LoadAccounts*)> &handler)
    {
        if (!m_is_still_open) {
            qCDebug(logger) << "Will not load accounts: storage no longer open";
            return;
        }

        LoadAccounts *job = new LoadAccounts(m_settings, m_secret);
        m_actions->queueAndProceed(job, [job, &handler](void) -> void
        {
            handler(job);
        });
    }

    Account * AccountStoragePrivate::acceptHotpAccount(const QUuid &id, const QString &name, const QString &issuer, const secrets::EncryptedSecret &secret, quint64 counter, int tokenLength, int offset, bool addChecksum)
    {
        Q_Q(AccountStorage);
        qCDebug(logger) << "Registering HOTP account:" << id;
        const std::function<Account*(AccountPrivate*)> registration([this, q, &id](AccountPrivate *p) -> Account *
        {
            Account *account = new Account(p, q);
            m_accounts.insert(id, account);
            return account;
        });
        m_ids.insert(id);
        m_names.insert(AccountPrivate::toFullName(name, issuer), id);
        m_accountsPrivate.insert(id, new AccountPrivate(registration, this, m_actions, id, name, issuer, secret, counter, tokenLength, offset, addChecksum));

        Q_ASSERT_X(m_accounts.contains(id), Q_FUNC_INFO, "account should have been registered");
        return m_accounts[id];
    }

    Account * AccountStoragePrivate::acceptTotpAccount(const QUuid &id, const QString &name, const QString &issuer, const secrets::EncryptedSecret &secret, uint timeStep, int tokenLength, const QDateTime &epoch, Account::Hash hash)
    {
        Q_Q(AccountStorage);
        qCDebug(logger) << "Registering TOTP account:" << id;
        const std::function<Account*(AccountPrivate*)> registration([this, q, &id](AccountPrivate *p) -> Account *
        {
            Account *account = new Account(p, q);
            m_accounts.insert(id, account);
            return account;
        });
        m_ids.insert(id);
        m_names.insert(AccountPrivate::toFullName(name, issuer), id);
        m_accountsPrivate.insert(id, new AccountPrivate(registration, this, m_actions, id, name, issuer, secret, epoch, timeStep, tokenLength, hash));

        Q_ASSERT_X(m_accounts.contains(id), Q_FUNC_INFO, "account should have been registered");
        return m_accounts[id];
    }

    bool AccountStoragePrivate::isLoaded(void) const
    {
        return m_is_loaded;
    }

    void AccountStoragePrivate::notifyLoaded(void)
    {
        Q_Q(AccountStorage);
        m_is_loaded = true;
        Q_EMIT q->loaded();
    }

    void AccountStoragePrivate::notifyError(void)
    {
        Q_Q(AccountStorage);
        m_has_error = true;
        Q_EMIT q->error();
    }

    void AccountStoragePrivate::clearError(void)
    {
        m_has_error = false;
    }

    bool AccountStoragePrivate::hasError(void) const
    {
        return m_has_error;
    }

    AccountStoragePrivate::AccountStoragePrivate(const SettingsProvider &settings, AccountSecret *secret, AccountStorage *storage, Dispatcher *dispatcher) :
        q_ptr(storage), m_is_loaded(false), m_has_error(false), m_is_still_open(true), m_actions(dispatcher), m_settings(settings), m_secret(secret)
    {
    }

    HandleCounterUpdate::HandleCounterUpdate(AccountPrivate *account, AccountStoragePrivate *storage, quint64 counter, SaveHotp *job, QObject *parent) :
        QObject(parent), m_accept_on_finish(true), m_counter(counter), m_account(account), m_storage(storage)
    {
        QObject::connect(job, &SaveHotp::invalid, this, &HandleCounterUpdate::rejected);
        QObject::connect(job, &SaveHotp::finished, this, &HandleCounterUpdate::finished);
    }

    void HandleCounterUpdate::rejected(void)
    {
        m_accept_on_finish = false;
        m_storage->notifyError();
    }

    void HandleCounterUpdate::finished(void)
    {
        if (m_accept_on_finish) {
            m_account->acceptCounter(m_counter);
        } else {
            qCWarning(logger)
                << "Rejecting counter update for account:" << m_account->id()
                << "Failed to save the updated account details to storage";
        }
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
