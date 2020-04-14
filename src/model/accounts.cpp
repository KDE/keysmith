/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "accounts.h"

#include "../logging_p.h"

#include <QtDebug>

KEYSMITH_LOGGER(logger, ".model.accounts")

namespace model
{
    qint64 millisecondsLeftForToken(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock)
    {
        QDateTime now = QDateTime::fromMSecsSinceEpoch(clock());
        if (!epoch.isValid() || !now.isValid() || timeStep == 0) {
            qCDebug(logger) << "Unable to compute milliseconds left: invalid arguments";
            return -1;
        }

        /*
         * Avoid integer overflow by casting to the wider type first before multiplying.
         * Not likely to happen 'in the wild', but good practice nevertheless
         */
        qint64 step = ((qint64) timeStep) * 1000LL;

        qint64 diff = epoch.msecsTo(now);

        /*
         * Compensate for the fact that % operator is not the same as mathematical mod in case diff is negative.
         * diff is negative when the given epoch is in the 'future' compared to the current clock value.
         */
        return diff < 0 ? - (diff % step) : step - (diff % step);
    }

    AccountView::AccountView(accounts::Account *model, QObject *parent) : QObject(parent), m_model(model)
    {
        QObject::connect(model, &accounts::Account::tokenChanged, this, &AccountView::tokenChanged);
        QObject::connect(this, &AccountView::remove, model, &accounts::Account::remove);
        QObject::connect(this, &AccountView::recompute, model, &accounts::Account::recompute);
        QObject::connect(this, &AccountView::advanceCounter, model, &accounts::Account::advanceCounter);
        QObject::connect(this, &AccountView::setCounter, model, &accounts::Account::setCounter);
    }

    bool AccountView::isHotp(void) const
    {
        return m_model->algorithm() == accounts::Account::Hotp;
    }

    bool AccountView::isTotp(void) const
    {
        return m_model->algorithm() == accounts::Account::Totp;
    }

    QString AccountView::name(void) const
    {
        return m_model->name();
    }

    QString AccountView::token(void) const
    {
        return m_model->token();
    }

    quint64 AccountView::counter(void) const
    {
        return m_model->counter();
    }

    uint AccountView::timeStep(void) const
    {
        return m_model->timeStep();
    }

    qint64 AccountView::millisecondsLeftForToken(void) const
    {
        if (!isTotp()) {
            qCDebug(logger) << "Unable to compute milliseconds left for token, wrong account type:" << m_model->algorithm();
            return -1;
        }

        return model::millisecondsLeftForToken(m_model->epoch(), m_model->timeStep());
    }

    SimpleAccountListModel::SimpleAccountListModel(accounts::AccountStorage *storage, QObject *parent) : QAbstractListModel(parent), m_storage(storage), m_index(QVector<QString>())
    {
        QObject::connect(storage, &accounts::AccountStorage::added, this, &SimpleAccountListModel::added);
        QObject::connect(storage, &accounts::AccountStorage::removed, this, &SimpleAccountListModel::removed);

        beginResetModel();
        for (const QString &name : m_storage->accounts()) {
            accounts::Account * existingAccount = m_storage->get(name);
            if (existingAccount) {
                m_index.append(name);
                m_accounts[name] = existingAccount;
                existingAccount->recompute();
            } else {
                qCDebug(logger) << "Account storage reported an account (name) but did not yield a valid account object";
            }
        }
        endResetModel();
    }

    void SimpleAccountListModel::addTotp(const QString &account, const QString &secret, uint timeStep, int tokenLength)
    {
        m_storage->addTotp(account, secret, timeStep, tokenLength);
    }

    void SimpleAccountListModel::addHotp(const QString &account, const QString &secret, quint64 counter, int tokenLength)
    {
        m_storage->addHotp(account, secret, counter, tokenLength);
    }

    QHash<int, QByteArray> SimpleAccountListModel::roleNames(void) const
    {
        QHash<int, QByteArray> roles;
        roles[NonStandardRoles::AccountRole] = "account";
        return roles;
    }

    QVariant SimpleAccountListModel::data(const QModelIndex &account, int role) const
    {
        if (!account.isValid()) {
            qCDebug(logger) << "Not returning any data, model index is invalid";
            return QVariant();
        }

        int accountIndex = account.row();
        if (accountIndex < 0 || m_index.size() < accountIndex) {
            qCDebug(logger) << "Not returning any data, model index is out of bounds:" << accountIndex << "model size is:" << m_index.size();
            return QVariant();
        }

        if (role != NonStandardRoles::AccountRole) {
            qCDebug(logger) << "Not returning any data, unknown role:" << role;
        }

        const QString accountName = m_index.at(accountIndex);
        accounts::Account * model = m_accounts.value(accountName, nullptr);
        if (model) {
            // assume QML ownership: don't worry about object lifecycle
            return QVariant::fromValue(new AccountView(model));
        } else {
            qCDebug(logger) << "Not returning any data, unable to find associated account model for:" << accountIndex;
            return QVariant();
        }
    }

    int SimpleAccountListModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return m_index.size();
    }

    void SimpleAccountListModel::added(const QString &account)
    {
        accounts::Account * newAccount = m_storage->get(account);
        if (!newAccount) {
            qCDebug(logger) << "Unable to handle added account: underlying storage did not return a valid object";
            return;
        }

        if (m_accounts.contains(account)) {
            qCDebug(logger) << "Added account already/still part of the model: requesting removal of the old one from the model first";
            removed(account);
        }

        int accountIndex = m_index.size();
        qCDebug(logger) << "Adding (new) account to the model at position:" << accountIndex;

        beginInsertRows(QModelIndex(), accountIndex, accountIndex);
        m_index.append(account);
        m_accounts[account] = newAccount;
        newAccount->recompute();
        endInsertRows();
    }

    void SimpleAccountListModel::removed(const QString &account)
    {
        int accountIndex = m_index.indexOf(account);
        if (accountIndex >= 0) {
            qCDebug(logger) << "Removing (old) account from the model at position:" << accountIndex;

            beginRemoveRows(QModelIndex(), accountIndex, accountIndex);
            m_index.remove(accountIndex);

            m_accounts.remove(account);
            endRemoveRows();
        } else {
            qCDebug(logger) << "Unable to handle account removal: account not part of the model";
        }
    }

    bool SimpleAccountListModel::isNameStillAvailable(const QString &account) const
    {
        return m_storage && m_storage->isNameStillAvailable(account);
    }

    AccountNameValidator::AccountNameValidator(QObject *parent) : QValidator(parent), m_accounts(nullptr), m_delegate(nullptr)
    {
    }

    QValidator::State AccountNameValidator::validate(QString &input, int &pos) const
    {
        if (!m_accounts) {
            qCDebug(logger) << "Unable to validat account name: missing accounts model object";
            return QValidator::Invalid;
        }

        QValidator::State result = m_delegate.validate(input, pos);
        return result != QValidator::Acceptable ||  m_accounts->isNameStillAvailable(input) ? result : QValidator::Intermediate;
    }

    void AccountNameValidator::fixup(QString &input) const
    {
        m_delegate.fixup(input);
    }

    SimpleAccountListModel * AccountNameValidator::accounts(void) const
    {
        return m_accounts;
    }

    void AccountNameValidator::setAccounts(SimpleAccountListModel *accounts)
    {
        if (accounts) {
            m_accounts = accounts;
            Q_EMIT accountsChanged();
        } else {
            qCDebug(logger) << "Ignoring new accounts model: not a valid object";
        }
    }
}
