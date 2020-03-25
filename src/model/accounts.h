/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef MODEL_ACCOUNTS_H
#define MODEL_ACCOUNTS_H

#include "../account/account.h"
#include "../validators/namevalidator.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QValidator>
#include <QVector>

namespace model
{
    qint64 millisecondsLeftForToken(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);

    class AccountView : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString name READ name NOTIFY never)
        Q_PROPERTY(QString token READ token NOTIFY tokenChanged)
        Q_PROPERTY(quint64 counter READ counter NOTIFY tokenChanged);
        Q_PROPERTY(uint timeStep READ timeStep NOTIFY never);
        Q_PROPERTY(bool isHotp READ isHotp NOTIFY never);
        Q_PROPERTY(bool isTotp READ isTotp NOTIFY never);
    public:
        explicit AccountView(accounts::Account *model, QObject *parent = nullptr);
        QString name(void) const;
        QString token(void) const;
        uint timeStep(void) const;
        quint64 counter(void) const;
        bool isHotp(void) const;
        bool isTotp(void) const;
        Q_INVOKABLE qint64 millisecondsLeftForToken(void) const;
    Q_SIGNALS:
        void never(void);
        void tokenChanged(void);
        void remove(void);
        void recompute(void);
        void advanceCounter(quint64 by = 1ULL);
        void setCounter(quint64 value);
    private:
        accounts::Account * const m_model;
    };

    class SimpleAccountListModel: public QAbstractListModel
    {
        Q_OBJECT
    public:
        enum NonStandardRoles {
            AccountRole = Qt::ItemDataRole::UserRole
        };
        Q_ENUM(NonStandardRoles)
    public:
        explicit SimpleAccountListModel(accounts::AccountStorage *storage, QObject *parent = nullptr);
        Q_INVOKABLE void addTotp(const QString &account, const QString &secret, uint timeStep, int tokenLength);
        Q_INVOKABLE void addHotp(const QString &account, const QString &secret, quint64 counter, int tokenLength);
        Q_INVOKABLE bool isNameStillAvailable(const QString &account) const;
        Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames(void) const override;
    private Q_SLOTS:
        void added(const QString &account);
        void removed(const QString &removed);
    private:
        accounts::AccountStorage * const m_storage;
    private:
        QVector<QString> m_index;
        QHash<QString, accounts::Account*> m_accounts;
    };

    class AccountNameValidator: public QValidator
    {
        Q_OBJECT
        Q_PROPERTY(model::SimpleAccountListModel * accounts READ accounts WRITE setAccounts NOTIFY accountsChanged);
    public:
        explicit AccountNameValidator(QObject *parent = nullptr);
        QValidator::State validate(QString &input, int &pos) const override;
        void fixup(QString &input) const override;
        SimpleAccountListModel * accounts(void) const;
        void setAccounts(SimpleAccountListModel *accounts);
    Q_SIGNALS:
        void accountsChanged(void);
    private:
        SimpleAccountListModel * m_accounts;
        validators::NameValidator m_delegate;
    };
}

Q_DECLARE_METATYPE(model::AccountView *);
Q_DECLARE_METATYPE(model::SimpleAccountListModel *);

#endif
