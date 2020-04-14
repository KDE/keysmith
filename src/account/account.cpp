/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account.h"
#include "account_p.h"
#include "actions_p.h"

#include "../logging_p.h"

#include <QTimer>

KEYSMITH_LOGGER(logger, ".accounts.account")

namespace accounts
{
    Account::Account(AccountPrivate *d, QObject *parent) : QObject(parent), m_dptr(d)
    {
    }

    QString Account::name(void) const
    {
        Q_D(const Account);
        return d->name();
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

    int Account::offset(void) const
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
        if(d->token().isEmpty() || d->algorithm() != Account::Hotp) {
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

    AccountStorage::AccountStorage(const SettingsProvider &settings, QThread *worker, QObject *parent) : QObject(parent), m_dptr(new AccountStoragePrivate(settings, this, new Dispatcher(worker, this)))
    {
        QTimer::singleShot(0, this, &AccountStorage::load);
    }

    AccountStorage * AccountStorage::open(const SettingsProvider &settings, QObject *parent)
    {
        QThread *worker = new QThread(parent);
        AccountStorage *storage = new AccountStorage(settings, worker, parent);

        QObject::connect(storage, &AccountStorage::disposed, worker, &QThread::quit);
        QObject::connect(worker, &QThread::finished, worker, &QThread::deleteLater);
        QObject::connect(worker, &QThread::destroyed, storage, &AccountStorage::deleteLater);
        worker->start();

        return storage;
    }

    void AccountStorage::load(void)
    {
        Q_D(AccountStorage);
        const std::function<void(LoadAccounts*)> handler([this](LoadAccounts *job) -> void
        {
            QObject::connect(job, &LoadAccounts::foundHotp, this, &AccountStorage::handleHotp);
            QObject::connect(job, &LoadAccounts::foundTotp, this, &AccountStorage::handleTotp);
        });
        d->load(handler);
    }

    bool AccountStorage::contains(const QString &name) const
    {
        Q_D(const AccountStorage);
        return d->contains(name);
    }

    Account * AccountStorage::get(const QString &name) const
    {
        Q_D(const AccountStorage);
        return d->get(name);
    }

    bool AccountStorage::isNameStillAvailable(const QString &name) const
    {
        Q_D(const AccountStorage);
        return d->isNameStillAvailable(name);
    }

    void AccountStorage::addHotp(const QString &name, const QString &secret, quint64 counter, int tokenLength, int offset, bool addChecksum)
    {
        Q_D(AccountStorage);
        const std::function<void(SaveHotp*)> handler([this](SaveHotp *job) -> void
        {
            QObject::connect(job, &SaveHotp::saved, this, &AccountStorage::handleHotp);
        });
        d->addHotp(handler, name, secret, counter, tokenLength, offset, addChecksum);
    }

    void AccountStorage::addTotp(const QString &name, const QString &secret, int timeStep, int tokenLength, const QDateTime &epoch, Account::Hash hash)
    {
        Q_D(AccountStorage);
        const std::function<void(SaveTotp*)> handler([this](SaveTotp *job) -> void
        {
            QObject::connect(job, &SaveTotp::saved, this, &AccountStorage::handleTotp);
        });
        d->addTotp(handler, name, secret, timeStep, tokenLength, epoch, hash);
    }

    void AccountStorage::accountRemoved(void)
    {
        Q_D(AccountStorage);

        QObject *from = sender();
        Account *account = from ? qobject_cast<Account*>(from) : nullptr;
        Q_ASSERT_X(account, Q_FUNC_INFO, "event should be sent by an account");

        const QString name = account->name();
        Q_EMIT removed(name);
        d->acceptAccountRemoval(name);
    }

    QVector<QString> AccountStorage::accounts(void) const
    {
        Q_D(const AccountStorage);
        return d->activeAccounts();
    }

    void AccountStorage::handleHotp(const QUuid id, const QString name, const QString secret, quint64 counter, int tokenLength)
    {
        Q_D(AccountStorage);
        if (!d->isStillOpen()) {
            qCDebug(logger)
                << "Not handling HOTP account:" << id
                << "Storage no longer open";
            return;
        }

        if (!isNameStillAvailable(name)) {
            qCDebug(logger)
                << "Not handling HOTP account:" << id
                << "Account name not available";
            return;
        }

        Account *accepted = d->acceptHotpAccount(id, name, secret, counter, tokenLength);
        QObject::connect(accepted, &Account::removed, this, &AccountStorage::accountRemoved);

        Q_EMIT added(name);
    }

    void AccountStorage::handleTotp(const QUuid id, const QString name, const QString secret, uint timeStep, int tokenLength)
    {
        Q_D(AccountStorage);
        if (!d->isStillOpen()) {
            qCDebug(logger)
                << "Not handling TOTP account:" << id
                << "Storage no longer open";
            return;
        }

        if (!isNameStillAvailable(name)) {
            qCDebug(logger)
                << "Not handling TOTP account:" << id
                << "Account name not available";
            return;
        }

        Account *accepted = d->acceptTotpAccount(id, name, secret, timeStep, tokenLength);
        QObject::connect(accepted, &Account::removed, this, &AccountStorage::accountRemoved);

        Q_EMIT added(name);
    }

    void AccountStorage::dispose(void)
    {
        Q_D(AccountStorage);
        d->dispose([this](Null *job) -> void
        {
            /*
             * Use destroyed() instead of finished() to guarantee the Null job has been disposed of before e.g. threads are cleaned up.
             * (If the QThread is disposed of before the Null job is cleaned up, the job would leak.)
             */
            QObject::connect(job, &Null::destroyed, this, &AccountStorage::handleDisposal);
        });
    }

    void AccountStorage::handleDisposal(void)
    {
        Q_D(AccountStorage);
        d->acceptDisposal();
    }
}
