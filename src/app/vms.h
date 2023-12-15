/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef APP_VMS_H
#define APP_VMS_H

#include "../account/account.h"
#include "../model/accounts.h"
#include "../model/password.h"
#include "keysmith.h"

namespace app
{
OverviewState *overviewStateOf(Keysmith *app);
FlowState *flowStateOf(Keysmith *app);
model::SimpleAccountListModel *accountListOf(Keysmith *app);

class AddAccountViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(model::AccountInput *input READ input CONSTANT)
    Q_PROPERTY(model::SimpleAccountListModel *accounts READ accounts CONSTANT)
    Q_PROPERTY(bool validateAvailability READ validateAvailability CONSTANT)
    Q_PROPERTY(bool quitEnabled READ quitEnabled CONSTANT)
public:
    explicit AddAccountViewModel(model::AccountInput *input,
                                 model::SimpleAccountListModel *accounts,
                                 bool quitEnabled,
                                 bool validateAvailability,
                                 QObject *parent = nullptr);
    model::AccountInput *input(void) const;
    model::SimpleAccountListModel *accounts(void) const;
    bool validateAvailability(void) const;
    bool quitEnabled(void) const;
Q_SIGNALS:
    void cancelled(void);
    void accepted(void);

private:
    model::AccountInput *const m_input;
    model::SimpleAccountListModel *const m_accounts;
    const bool m_quitEnabled;
    const bool m_validateAvailability;
};

class RenameAccountViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(model::AccountInput *input READ input CONSTANT)
    Q_PROPERTY(model::SimpleAccountListModel *accounts READ accounts CONSTANT)
public:
    explicit RenameAccountViewModel(model::AccountInput *input, model::SimpleAccountListModel *accounts, QObject *parent = nullptr);
    model::AccountInput *input(void) const;
    model::SimpleAccountListModel *accounts(void) const;
Q_SIGNALS:
    void cancelled(void);
    void accepted(void);

private:
    model::AccountInput *const m_input;
    model::SimpleAccountListModel *const m_accounts;
};

class ErrorViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString errorText READ error CONSTANT)
    Q_PROPERTY(QString errorTitle READ title CONSTANT)
    Q_PROPERTY(bool quitEnabled READ quitEnabled CONSTANT)
public:
    explicit ErrorViewModel(const QString &errorTitle, const QString &errorText, bool quitEnabled, QObject *parent = nullptr);
    bool quitEnabled(void) const;
    QString error(void) const;
    QString title(void) const;
Q_SIGNALS:
    void dismissed(void);

private:
    const QString m_title;
    const QString m_error;
    const bool m_quitEnabled;
};

class ImportAccountViewModel: public QObject
{
    Q_OBJECT
    Q_PROPERTY(model::ImportInput * input READ input CONSTANT)
    Q_PROPERTY(model::SimpleAccountListModel * accounts READ accounts CONSTANT)
    Q_PROPERTY(bool validateAvailability READ validateAvailability CONSTANT)
    Q_PROPERTY(bool quitEnabled READ quitEnabled CONSTANT)
public:
    explicit ImportAccountViewModel(model::ImportInput *input, model::SimpleAccountListModel *accounts,
                                    bool quitEnabled, bool validateAvailability,
                                    QObject *parent = nullptr);
    model::ImportInput * input(void) const;
    model::SimpleAccountListModel * accounts(void) const;
    bool validateAvailability(void) const;
    bool quitEnabled(void) const;
Q_SIGNALS:
    void cancelled(void);
    void accepted(void);
private:
    model::ImportInput * const m_input;
    model::SimpleAccountListModel * const m_accounts;
    const bool m_quitEnabled;
    const bool m_validateAvailability;
};

class PasswordViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool failed READ failed NOTIFY failedChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    explicit PasswordViewModel(model::PasswordRequest *request, QObject *parent = nullptr);
    virtual ~PasswordViewModel();
    bool failed(void) const;
    bool busy(void) const;
Q_SIGNALS:
    void failedChanged(void);
    void busyChanged(void);
private Q_SLOTS:
    void rejected(void);

protected:
    bool m_failed;
    model::PasswordRequest *const m_request;
};

class SetupPasswordViewModel : public PasswordViewModel
{
    Q_OBJECT
public:
    explicit SetupPasswordViewModel(model::PasswordRequest *request, QObject *parent = nullptr);
public Q_SLOTS:
    void setup(const QString &password, const QString &confirmedPassword);
};

class UnlockAccountsViewModel : public PasswordViewModel
{
    Q_OBJECT
public:
    explicit UnlockAccountsViewModel(model::PasswordRequest *request, QObject *parent = nullptr);
public Q_SLOTS:
    void unlock(const QString &password);
};

class AccountsOverviewViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(model::SimpleAccountListModel *accounts READ accounts CONSTANT)
    Q_PROPERTY(bool actionsEnabled READ actionsEnabled NOTIFY actionsEnabledChanged)
public:
    explicit AccountsOverviewViewModel(Keysmith *app);
    model::SimpleAccountListModel *accounts(void) const;
    bool actionsEnabled(void) const;
public Q_SLOTS:
    void addNewAccount(void);
    void importAccount(void);
Q_SIGNALS:
    void actionsEnabledChanged(void);

private:
    Keysmith *const m_app;
    model::SimpleAccountListModel *const m_accounts;
};
}

#endif
