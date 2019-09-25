/*****************************************************************************
 * Copyright: 2013 Michael Zanetti <michael_zanetti@gmx.net>                 *
 *                                                                           *
 * This project is free software: you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This project is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#include "accountmodel.h"

#include "account.h"

#include <QSettings>
#include <QStringList>
//#include <QDebug>

AccountModel::AccountModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QSettings settings("org.kde.otpclient", "otpclient");
    const QStringList entries = settings.childGroups();
//    qDebug() << "loading settings file:" << settings.fileName();
    for(const QString &group : entries) {
//        qDebug() << "found group" << group << QUuid(group).toString();

        QUuid id(group);

        settings.beginGroup(group);
        Account *account = new Account(id, this);
        account->setName(settings.value("account").toString());
        account->setType(settings.value("type", "hotp").toString() == "totp" ? Account::TypeTOTP : Account::TypeHOTP);
        account->setSecret(settings.value("secret").toString());
        account->setCounter(settings.value("counter").toInt());
        account->setTimeStep(settings.value("timeStep").toInt());
        account->setPinLength(settings.value("pinLength").toInt());

        m_accounts.append(account);
        wireAccount(account);
        settings.endGroup();
    }
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_accounts.count();
}

void AccountModel::wireAccount(const Account *account)
{
    const auto h = &AccountModel::accountChanged;
    QObject::connect(account, &Account::nameChanged, this, h);
    QObject::connect(account, &Account::typeChanged, this, h);
    QObject::connect(account, &Account::secretChanged, this, h);
    QObject::connect(account, &Account::counterChanged, this, h);
    QObject::connect(account, &Account::pinLengthChanged, this, h);
    QObject::connect(account, &Account::otpChanged, this, h);
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case RoleName:
        return m_accounts.at(index.row())->name();
    case RoleType:
        return m_accounts.at(index.row())->type();
    case RoleSecret:
        return m_accounts.at(index.row())->secret();
    case RoleCounter:
        return m_accounts.at(index.row())->counter();
    case RoleTimeStep:
        return m_accounts.at(index.row())->timeStep();
    case RolePinLength:
        return m_accounts.at(index.row())->pinLength();
    case RoleOtp:
        return m_accounts.at(index.row())->otp();
    }

    return QVariant();
}

Account *AccountModel::get(int index) const
{
    if (index > -1 && m_accounts.count() > index) {
        return m_accounts.at(index);
    }
    return nullptr;
}

Account *AccountModel::createAccount()
{
    Account *account = new Account(QUuid::createUuid(), this);
    beginInsertRows(QModelIndex(), m_accounts.count(), m_accounts.count());
    m_accounts.append(account);

    wireAccount(account);
    storeAccount(account);

    endInsertRows();
    return account;
}

void AccountModel::deleteAccount(int index)
{
//    qDebug() << "starting deleteAccount" << index << m_accounts.count();
    beginRemoveRows(QModelIndex(), index, index);

    Account *account = m_accounts.takeAt(index);
//    qDebug() << "got account" << account;
    QSettings settings("org.kde.otpclient", "otpclient");
    settings.beginGroup(account->id().toString());
    settings.remove("");
    settings.endGroup();

//    qDebug() << "removed from settings";
    account->deleteLater();

    endRemoveRows();
//    qDebug() << "done with deleteAccount";
}

void AccountModel::deleteAccount(Account *account)
{
    int index = m_accounts.indexOf(account);
    deleteAccount(index);
}

QHash<int, QByteArray> AccountModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleName, "name");
    roles.insert(RoleType, "type");
    roles.insert(RoleSecret, "secret");
    roles.insert(RoleCounter, "counter");
    roles.insert(RoleTimeStep, "timeStep");
    roles.insert(RolePinLength, "pinLength");
    roles.insert(RoleOtp, "otp");
    return roles;
}

void AccountModel::generateNext(int account)
{
    m_accounts.at(account)->next();
    Q_EMIT dataChanged(index(account), index(account), QVector<int>() << RoleCounter << RoleOtp);
}

void AccountModel::refresh()
{
    Q_EMIT beginResetModel();
    Q_EMIT endResetModel();
}

void AccountModel::accountChanged()
{
    Account *account = qobject_cast<Account*>(sender());
    storeAccount(account);

//    qDebug() << "account changed";
    int accountIndex = m_accounts.indexOf(account);
    Q_EMIT dataChanged(index(accountIndex), index(accountIndex));
}

void AccountModel::storeAccount(const Account *account)
{
    QSettings settings("org.kde.otpclient", "otpclient");
    settings.beginGroup(account->id().toString());
    settings.setValue("account", account->name());
    settings.setValue("type", account->type() == Account::TypeTOTP ? "totp" : "hotp");
    settings.setValue("secret", account->secret());
    settings.setValue("counter", account->counter());
    settings.setValue("timeStep", account->timeStep());
    settings.setValue("pinLength", account->pinLength());
    settings.endGroup();
//    qDebug() << "saved to" << settings.fileName();

}
