/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef APP_KEYSMITH_H
#define APP_KEYSMITH_H

#include "../account/account.h"
#include "../model/accounts.h"
#include "../model/password.h"

#include <QMetaEnum>
#include <QObject>
#include <QQmlEngine>
#include <QSharedPointer>

namespace app
{
class Navigation : public QObject
{
    Q_OBJECT
public:
    enum Page {
        Error,
        AddAccount,
        ImportAccount,
        RenameAccount,
        AccountsOverview,
        SetupPassword,
        UnlockAccounts,
        ScanQR,
    };
    Q_ENUM(Page)
public:
    explicit Navigation(QQmlEngine *const engine);
    Q_INVOKABLE QString name(app::Navigation::Page page) const;
public Q_SLOTS:
    void push(app::Navigation::Page page, QObject *modelToTransfer);
    void navigate(app::Navigation::Page page, QObject *modelToTransfer);
    void pop();

Q_SIGNALS:
    void routed(app::Navigation::Page route, QObject *transferred);
    void pushed(app::Navigation::Page route, QObject *transferred);
    void popped(void);

private:
    QQmlEngine *const m_engine;
};

class OverviewState;
class FlowState;

class Store
{
public:
    explicit Store(void);
    accounts::AccountStorage *accounts(void) const;
    model::SimpleAccountListModel *accountList(void) const;
    OverviewState *overview(void) const;
    FlowState *flows(void) const;

private:
    QSharedPointer<FlowState> m_flows;
    QSharedPointer<OverviewState> m_overview;
    QSharedPointer<accounts::AccountStorage> m_accounts;
    QSharedPointer<model::SimpleAccountListModel> m_accountList;
};

class Keysmith : public QObject
{
    Q_OBJECT
    Q_PROPERTY(app::Navigation *navigation READ navigation CONSTANT)
public:
    explicit Keysmith(Navigation *const navigation, QObject *parent = nullptr);
    virtual ~Keysmith();
    const Store &store(void) const;
    Navigation *navigation(void) const;
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE model::SimpleAccountListModel *accountListModel(void);
    Q_INVOKABLE model::PasswordRequest *passwordRequest(void);

private:
    accounts::AccountStorage *storage(void);

private:
    Store m_store;
    Navigation *const m_navigation;
};
}

Q_DECLARE_METATYPE(app::Navigation *);

#endif
