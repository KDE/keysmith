/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "vms.h"
#include "flows_p.h"
#include "state_p.h"

namespace app
{
AddAccountViewModel::AddAccountViewModel(model::AccountInput *input,
                                         model::SimpleAccountListModel *accounts,
                                         bool quitEnabled,
                                         bool validateAvailability,
                                         QObject *parent)
    : QObject(parent)
    , m_input(input)
    , m_accounts(accounts)
    , m_quitEnabled(quitEnabled)
    , m_validateAvailability(validateAvailability)
{
}

model::AccountInput *AddAccountViewModel::input(void) const
{
    return m_input;
}

model::SimpleAccountListModel *AddAccountViewModel::accounts(void) const
{
    return m_accounts;
}

bool AddAccountViewModel::validateAvailability(void) const
{
    return m_validateAvailability;
}

bool AddAccountViewModel::quitEnabled(void) const
{
    return m_quitEnabled;
}

RenameAccountViewModel::RenameAccountViewModel(model::AccountInput *input, model::SimpleAccountListModel *accounts, QObject *parent)
    : QObject(parent)
    , m_input(input)
    , m_accounts(accounts)
{
}

model::AccountInput *RenameAccountViewModel::input(void) const
{
    return m_input;
}

model::SimpleAccountListModel *RenameAccountViewModel::accounts(void) const
{
    return m_accounts;
}

ErrorViewModel::ErrorViewModel(const QString &errorTitle, const QString &errorText, bool quitEnabled, QObject *parent)
    : QObject(parent)
    , m_title(errorTitle)
    , m_error(errorText)
    , m_quitEnabled(quitEnabled)
{
}

QString ErrorViewModel::error(void) const
{
    return m_error;
}

QString ErrorViewModel::title(void) const
{
    return m_title;
}

bool ErrorViewModel::quitEnabled(void) const
{
    return m_quitEnabled;
}

OverviewState *overviewStateOf(Keysmith *app)
{
    auto overview = app->store().overview();
    Q_ASSERT_X(overview, Q_FUNC_INFO, "should have a valid overview state object");
    return overview;
}

FlowState *flowStateOf(Keysmith *app)
{
    auto flows = app->store().flows();
    Q_ASSERT_X(flows, Q_FUNC_INFO, "should have a valid flow state object");
    return flows;
}

model::SimpleAccountListModel *accountListOf(Keysmith *app)
{
    Q_ASSERT_X(app, Q_FUNC_INFO, "should have a Keysmith application instance");

    auto list = app->store().accountList();

    Q_ASSERT_X(list, Q_FUNC_INFO, "should have an SimpleAccountListModel instance");
    return list;
}

AccountsOverviewViewModel::AccountsOverviewViewModel(Keysmith *app)
    : QObject()
    , m_app(app)
    , m_accounts(accountListOf(app))
{
    QObject::connect(overviewStateOf(m_app), &OverviewState::actionsEnabledChanged, this, &AccountsOverviewViewModel::actionsEnabledChanged);
}

model::SimpleAccountListModel *AccountsOverviewViewModel::accounts(void) const
{
    return m_accounts;
}

bool AccountsOverviewViewModel::actionsEnabled(void) const
{
    return overviewStateOf(m_app)->actionsEnabled();
}

void AccountsOverviewViewModel::addNewAccount(void)
{
    auto flow = new ManualAddAccountFlow(m_app);
    flow->run();
}

PasswordViewModel::PasswordViewModel(model::PasswordRequest *request, QObject *parent)
    : QObject(parent)
    , m_failed(false)
    , m_request(request)
{
    Q_ASSERT_X(request, Q_FUNC_INFO, "should have a valid password request object");
    QObject::connect(m_request, &model::PasswordRequest::passwordRejected, this, &PasswordViewModel::rejected);
    QObject::connect(m_request, &model::PasswordRequest::passwordStateChanged, this, &PasswordViewModel::busyChanged);
}

PasswordViewModel::~PasswordViewModel()
{
}

bool PasswordViewModel::busy(void) const
{
    return m_request->passwordProvided();
}

bool PasswordViewModel::failed(void) const
{
    return m_failed;
}

void PasswordViewModel::rejected(void)
{
    if (!m_failed) {
        m_failed = true;
        Q_EMIT failedChanged();
    }
}

SetupPasswordViewModel::SetupPasswordViewModel(model::PasswordRequest *request, QObject *parent)
    : PasswordViewModel(request, parent)
{
}

void SetupPasswordViewModel::setup(const QString &password, const QString &confirmedPassword)
{
    bool result = !m_request->provideBothPasswords(password, confirmedPassword);
    if (result != m_failed) {
        m_failed = result;
        Q_EMIT failedChanged();
    }
}

UnlockAccountsViewModel::UnlockAccountsViewModel(model::PasswordRequest *request, QObject *parent)
    : PasswordViewModel(request, parent)
{
}

void UnlockAccountsViewModel::unlock(const QString &password)
{
    bool result = !m_request->providePassword(password);
    if (result != m_failed) {
        m_failed = result;
        Q_EMIT failedChanged();
    }
}
}

#include "moc_vms.cpp"
