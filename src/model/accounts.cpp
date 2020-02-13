/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "accounts.h"

#include <QtDebug>

namespace model
{
    qint64 millisecondsLeftForToken(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock)
    {
        QDateTime now = QDateTime::fromMSecsSinceEpoch(clock());
        if (epoch.isValid() && now.isValid() && timeStep > 0) {
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
        // TODO: warn if not

        return -1;
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
        if (isTotp()) {
            return model::millisecondsLeftForToken(m_model->epoch(), m_model->timeStep());
        }
        // TODO: warn if not
        return -1;
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
            }
            // TODO: warn if not
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
            // TODO warn about this
            return QVariant();
        }

        int accountIndex = account.row();
        if (accountIndex < 0 || m_index.size() < accountIndex) {
            // TODO warn about this
            return QVariant();
        }

        if (role == NonStandardRoles::AccountRole) {
            const QString accountName = m_index.at(accountIndex);
            accounts::Account * model = m_accounts.value(accountName, nullptr);
            if (model) {
                // assume QML ownership: don't worry about object lifecycle
                return QVariant::fromValue(new AccountView(model));
            }
        }

        // TODO warn about this
        return QVariant();
    }

    int SimpleAccountListModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return m_index.size();
    }

    void SimpleAccountListModel::added(const QString &account)
    {
        accounts::Account * newAccount = m_storage->get(account);
        if (newAccount) {
            if (m_accounts.contains(account)) {
                //TODO warn about this
                removed(account);
            }

            int accountIndex = m_index.size();
            beginInsertRows(QModelIndex(), accountIndex, accountIndex);
            m_index.append(account);
            m_accounts[account] = newAccount;
            newAccount->recompute();
            endInsertRows();
        }
        // TODO: warn if not
    }

    void SimpleAccountListModel::removed(const QString &account)
    {
        int accountIndex = m_index.indexOf(account);
        if (accountIndex >= 0) {
            beginRemoveRows(QModelIndex(), accountIndex, accountIndex);
            m_index.remove(accountIndex);

            m_accounts.remove(account);
            endRemoveRows();
        }
        // TODO: warn if not
    }

}
