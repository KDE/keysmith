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

#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H

#include <QAbstractListModel>

class Account;

class AccountModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        RoleName,
        RoleType,
        RoleSecret,
        RoleCounter,
        RoleTimeStep,
        RolePinLength,
        RoleOtp
    };

    explicit AccountModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Account *get(int index) const;
    Q_INVOKABLE Account *createAccount();
    Q_INVOKABLE void deleteAccount(int index);
    Q_INVOKABLE void deleteAccount(Account *account);

public Q_SLOTS:
    void generateNext(int account);
    void refresh();

private Q_SLOTS:
    void accountChanged();
    void storeAccount(const Account *account);

private:
    void wireAccount(const Account *account);

private:
    QList<Account*> m_accounts;
};

#endif // ACCOUNTMODEL_H
