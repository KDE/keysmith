/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account.h"
#include "account_p.h"
#include "actions_p.h"

#include "../logging_p.h"

#include <QTimer>

KEYSMITH_LOGGER(logger, ".accounts.account")

namespace accounts
{
Account::Account(AccountPrivate *d, QObject *parent)
    : QObject(parent)
    , m_dptr(d)
{
}

QString Account::name(void) const
{
    Q_D(const Account);
    return d->name();
}

QString Account::issuer(void) const
{
    Q_D(const Account);
    return d->issuer();
}

QString Account::token(void) const
{
    Q_D(const Account);
    return d->token();
}

quint64 Account::counter(void) const
{
    Q_D(const Account);
    return d->counter();
}

QDateTime Account::epoch(void) const
{
    Q_D(const Account);
    return d->epoch();
}

uint Account::timeStep(void) const
{
    Q_D(const Account);
    return d->timeStep();
}

std::optional<uint> Account::offset(void) const
{
    Q_D(const Account);
    return d->offset();
}

int Account::tokenLength(void) const
{
    Q_D(const Account);
    return d->tokenLength();
}

bool Account::checksum(void) const
{
    Q_D(const Account);
    return d->checksum();
}

Account::Hash Account::hash(void) const
{
    Q_D(const Account);
    return d->hash();
}

Account::Algorithm Account::algorithm(void) const
{
    Q_D(const Account);
    return d->algorithm();
}

void Account::recompute(void)
{
    Q_D(Account);
    if (d->token().isEmpty() || d->algorithm() != Account::Hotp) {
        d->recompute();
    }
}

void Account::setCounter(quint64 value)
{
    Q_D(Account);
    d->setCounter(value);
}

void Account::advanceCounter(quint64 by)
{
    setCounter(counter() + by);
}

void Account::remove(void)
{
    Q_D(Account);
    d->remove();
}

AccountStorage::AccountStorage(const SettingsProvider &settings, QThread *worker, AccountSecret *secret, QObject *parent)
    : QObject(parent)
    , m_dptr(new AccountStoragePrivate(settings, secret ? secret : new AccountSecret(secrets::defaultSecureRandom, this), this, new Dispatcher(worker, this)))
{
    QTimer::singleShot(0, this, &AccountStorage::unlock);
}

AccountStorage *AccountStorage::open(const SettingsProvider &settings, AccountSecret *secret, QObject *parent)
{
    QThread *worker = new QThread(parent);
    AccountStorage *storage = new AccountStorage(settings, worker, secret, parent);

    QObject::connect(storage, &AccountStorage::disposed, worker, &QThread::quit);
    QObject::connect(worker, &QThread::finished, worker, &QThread::deleteLater);
    QObject::connect(worker, &QThread::destroyed, storage, &AccountStorage::deleteLater);
    worker->start();

    return storage;
}

void AccountStorage::unlock(void)
{
    Q_D(AccountStorage);
    const std::function<void(RequestAccountPassword *)> handler([this](RequestAccountPassword *job) -> void {
        QObject::connect(job, &RequestAccountPassword::unlocked, this, &AccountStorage::load);
        QObject::connect(job, &RequestAccountPassword::failed, this, &AccountStorage::handleError);
    });
    d->unlock(handler);
}

void AccountStorage::load(void)
{
    Q_D(AccountStorage);
    const std::function<void(LoadAccounts *)> handler([this](LoadAccounts *job) -> void {
        QObject::connect(job, &LoadAccounts::foundHotp, this, &AccountStorage::handleHotp);
        QObject::connect(job, &LoadAccounts::foundTotp, this, &AccountStorage::handleTotp);
        QObject::connect(job, &LoadAccounts::finished, this, &AccountStorage::handleLoaded);
        QObject::connect(job, &LoadAccounts::failedToLoadAllAccounts, this, &AccountStorage::handleError);
    });
    d->load(handler);
}

bool AccountStorage::contains(const QString &fullName) const
{
    Q_D(const AccountStorage);
    return d->contains(fullName);
}

bool AccountStorage::contains(const QString &name, const QString &issuer) const
{
    return contains(AccountPrivate::toFullName(name, issuer));
}

Account *AccountStorage::get(const QString &fullName) const
{
    Q_D(const AccountStorage);
    return d->get(fullName);
}

Account *AccountStorage::get(const QString &name, const QString &issuer) const
{
    return get(AccountPrivate::toFullName(name, issuer));
}

AccountSecret *AccountStorage::secret(void) const
{
    Q_D(const AccountStorage);
    return d->secret();
}

bool AccountStorage::isAccountStillAvailable(const QString &fullName) const
{
    Q_D(const AccountStorage);
    return d->isAccountStillAvailable(fullName);
}

bool AccountStorage::isAccountStillAvailable(const QString &name, const QString &issuer) const
{
    return isAccountStillAvailable(AccountPrivate::toFullName(name, issuer));
}

void AccountStorage::addHotp(const QString &name,
                             const QString &issuer,
                             const QString &secret,
                             uint tokenLength,
                             quint64 counter,
                             const std::optional<uint> offset,
                             bool addChecksum)
{
    Q_D(AccountStorage);
    const std::function<void(SaveHotp *)> handler([this](SaveHotp *job) -> void {
        QObject::connect(job, &SaveHotp::saved, this, &AccountStorage::handleHotp);
        QObject::connect(job, &SaveHotp::invalid, this, &AccountStorage::handleError);
    });
    if (!d->addHotp(handler, name, issuer.isEmpty() ? QString() : issuer, secret, tokenLength, counter, offset, addChecksum)) {
        Q_EMIT error();
    }
}

void AccountStorage::addTotp(const QString &name,
                             const QString &issuer,
                             const QString &secret,
                             uint tokenLength,
                             uint timeStep,
                             const QDateTime &epoch,
                             Account::Hash hash)
{
    Q_D(AccountStorage);
    const std::function<void(SaveTotp *)> handler([this](SaveTotp *job) -> void {
        QObject::connect(job, &SaveTotp::saved, this, &AccountStorage::handleTotp);
        QObject::connect(job, &SaveTotp::invalid, this, &AccountStorage::handleError);
    });
    if (!d->addTotp(handler, name, issuer.isEmpty() ? QString() : issuer, secret, tokenLength, timeStep, epoch, hash)) {
        Q_EMIT error();
    }
}

void AccountStorage::accountRemoved(void)
{
    Q_D(AccountStorage);

    QObject *from = sender();
    Account *account = from ? qobject_cast<Account *>(from) : nullptr;
    Q_ASSERT_X(account, Q_FUNC_INFO, "event should be sent by an account");

    const QString fullName = AccountPrivate::toFullName(account->name(), account->issuer());
    d->acceptAccountRemoval(fullName);
    Q_EMIT removed(fullName);
}

QVector<QString> AccountStorage::accounts(void) const
{
    Q_D(const AccountStorage);
    return d->activeAccounts();
}

void AccountStorage::handleHotp(const QUuid id,
                                const QString &name,
                                const QString &issuer,
                                const QByteArray &secret,
                                const QByteArray &nonce,
                                uint tokenLength,
                                quint64 counter,
                                bool fixedTruncation,
                                uint offset,
                                bool checksum)
{
    Q_D(AccountStorage);
    if (!d->isStillOpen()) {
        qCDebug(logger) << "Not handling HOTP account:" << id << "Storage no longer open";
        return;
    }

    if (!isAccountStillAvailable(name, issuer)) {
        qCDebug(logger) << "Not handling HOTP account:" << id << "Account name or issuer not available";
        return;
    }

    std::optional<secrets::EncryptedSecret> encryptedSecret = secrets::EncryptedSecret::from(secret, nonce);
    if (!encryptedSecret) {
        qCDebug(logger) << "Not handling HOTP account:" << id << "Invalid encrypted secret/nonce";
        return;
    }

    const std::optional<uint> offsetValue = fixedTruncation ? std::optional<uint>((uint)offset) : std::nullopt;
    Account *accepted = d->acceptHotpAccount(id, name, issuer, *encryptedSecret, tokenLength, counter, offsetValue, checksum);
    QObject::connect(accepted, &Account::removed, this, &AccountStorage::accountRemoved);

    Q_EMIT added(AccountPrivate::toFullName(name, issuer));
}

void AccountStorage::handleTotp(const QUuid id,
                                const QString &name,
                                const QString &issuer,
                                const QByteArray &secret,
                                const QByteArray &nonce,
                                uint tokenLength,
                                uint timeStep,
                                const QDateTime &epoch,
                                Account::Hash hash)
{
    Q_D(AccountStorage);
    if (!d->isStillOpen()) {
        qCDebug(logger) << "Not handling TOTP account:" << id << "Storage no longer open";
        return;
    }

    if (!isAccountStillAvailable(name, issuer)) {
        qCDebug(logger) << "Not handling TOTP account:" << id << "Account name or issuer not available";
        return;
    }

    std::optional<secrets::EncryptedSecret> encryptedSecret = secrets::EncryptedSecret::from(secret, nonce);
    if (!encryptedSecret) {
        qCDebug(logger) << "Not handling TOTP account:" << id << "Invalid encrypted secret/nonce";
        return;
    }

    Account *accepted = d->acceptTotpAccount(id, name, issuer, *encryptedSecret, tokenLength, timeStep, epoch, hash);
    QObject::connect(accepted, &Account::removed, this, &AccountStorage::accountRemoved);

    Q_EMIT added(AccountPrivate::toFullName(name, issuer));
}

void AccountStorage::dispose(void)
{
    Q_D(AccountStorage);
    d->dispose([this](Null *job) -> void {
        /*
         * Use destroyed() instead of finished() to guarantee the Null job has been disposed of before e.g. threads
         * are cleaned up. If the QThread is disposed of before the Null job is cleaned up, the job would leak.
         */
        QObject::connect(job, &Null::destroyed, this, &AccountStorage::handleDisposal);
    });
}

void AccountStorage::handleDisposal(void)
{
    Q_D(AccountStorage);
    d->acceptDisposal();
}

bool AccountStorage::hasError(void) const
{
    Q_D(const AccountStorage);
    return d->hasError();
}

void AccountStorage::clearError(void)
{
    Q_D(AccountStorage);
    d->clearError();
}

void AccountStorage::handleError(void)
{
    Q_D(AccountStorage);
    d->notifyError();
}

void AccountStorage::handleLoaded(void)
{
    Q_D(AccountStorage);
    d->notifyLoaded();
}

bool AccountStorage::isLoaded(void) const
{
    Q_D(const AccountStorage);
    return d->isLoaded();
}
}

#include "moc_account.cpp"
