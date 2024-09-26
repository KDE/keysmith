/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef MODEL_ACCOUNTS_H
#define MODEL_ACCOUNTS_H

#include "../account/account.h"
#include "../validators/namevalidator.h"
#include "input.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QValidator>
#include <QVector>

#include <optional>

namespace model
{
qint64 millisecondsLeftForToken(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);

class AccountView : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isHotp READ isHotp CONSTANT)
    Q_PROPERTY(bool isTotp READ isTotp CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)
    Q_PROPERTY(QString issuer READ issuer CONSTANT)
    Q_PROPERTY(QString secret READ secret CONSTANT)
    Q_PROPERTY(quint64 counter READ counter NOTIFY tokenChanged)
    Q_PROPERTY(QDateTime epoch READ epoch CONSTANT)
    Q_PROPERTY(uint timeStep READ timeStep CONSTANT)
    Q_PROPERTY(uint offset READ offset CONSTANT)
    Q_PROPERTY(int tokenLength READ tokenLength CONSTANT)
    Q_PROPERTY(QString hash READ hash CONSTANT)
public:
    explicit AccountView(accounts::Account *model, QObject *parent = nullptr);
    bool isHotp(void) const;
    bool isTotp(void) const;
    QString name(void) const;
    QString token(void) const;
    QString issuer(void) const;
    QString secret(void) const;
    quint64 counter(void) const;
    QDateTime epoch(void) const;
    uint timeStep(void) const;
    uint offset(void) const;
    int tokenLength(void) const;
    QString hash(void) const;
    Q_INVOKABLE qint64 millisecondsLeftForToken(void) const;
Q_SIGNALS:
    void tokenChanged(void);
    void remove(void);
    void recompute(void);
    void advanceCounter(quint64 by = 1ULL);
    void setCounter(quint64 value);

private:
    accounts::Account *const m_model;
};

class SimpleAccountListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
    Q_PROPERTY(bool error READ error WRITE setError NOTIFY errorChanged)
public:
    enum NonStandardRoles { AccountRole = Qt::ItemDataRole::UserRole };
    Q_ENUM(NonStandardRoles)
    enum TOTPAlgorithms { Sha1, Sha256, Sha512 };
    Q_ENUM(TOTPAlgorithms)
private:
    static accounts::Account::Hash toHash(const TOTPAlgorithms value);

public:
    explicit SimpleAccountListModel(accounts::AccountStorage *storage, QObject *parent = nullptr);
    Q_INVOKABLE void addAccount(model::AccountInput *input);
    Q_INVOKABLE bool isAccountStillAvailable(const QString &name, const QString &issuer) const;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames(void) const override;

public:
    bool loaded(void) const;
    bool error(void) const;
    void setError(bool markAsError);
Q_SIGNALS:
    void loadedChanged(void);
    void errorChanged(void);
private Q_SLOTS:
    void added(const QString &account);
    void removed(const QString &removed);
    void handleError(void);

private:
    AccountView *createView(const QString &name);
    void populate(const QString &name, AccountView *account);

private:
    accounts::AccountStorage *const m_storage;

private:
    bool m_has_error;
    QVector<QString> m_index;
    QHash<QString, AccountView *> m_accounts;
};

class SortedAccountsListModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SortedAccountsListModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *sourceModel) override;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

class AccountNameValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(QString issuer READ issuer WRITE setIssuer NOTIFY issuerChanged)
    Q_PROPERTY(model::SimpleAccountListModel *accounts READ accounts WRITE setAccounts NOTIFY accountsChanged)
    Q_PROPERTY(bool validateAvailability READ validateAvailability WRITE setValidateAvailability NOTIFY validateAvailabilityChanged)
public:
    explicit AccountNameValidator(QObject *parent = nullptr);
    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &input) const override;
    bool validateAvailability(void) const;
    void setValidateAvailability(bool enabled);
    QString issuer(void) const;
    void setIssuer(const QString &issuer);
    SimpleAccountListModel *accounts(void) const;
    void setAccounts(SimpleAccountListModel *accounts);
Q_SIGNALS:
    void issuerChanged(void);
    void accountsChanged(void);
    void validateAvailabilityChanged(void);

private:
    bool m_validateAvailability;
    std::optional<QString> m_issuer;
    SimpleAccountListModel *m_accounts;
    const validators::NameValidator m_delegate;
};
}

Q_DECLARE_METATYPE(model::AccountView *);
Q_DECLARE_METATYPE(model::SimpleAccountListModel *);

#endif
