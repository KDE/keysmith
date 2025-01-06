/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 */

#include "flows_p.h"

#include "../model/qr.h"
#include "cli.h"
#include "state_p.h"
#include "vms.h"

#include "../logging_p.h"

#include <KLocalizedString>
#include <QTimer>

KEYSMITH_LOGGER(logger, ".app.flows_p")

namespace app
{
accounts::AccountStorage *storageOf(Keysmith *app)
{
    Q_ASSERT_X(app, Q_FUNC_INFO, "should have a Keysmith application instance");

    auto accounts = app->store().accounts();

    Q_ASSERT_X(accounts, Q_FUNC_INFO, "should have an AccountStorage instance");
    return accounts;
}

Navigation *navigationFor(Keysmith *app)
{
    auto nav = app->navigation();
    Q_ASSERT_X(nav, Q_FUNC_INFO, "should have a Navigation instance");
    return nav;
}

InitialFlow::InitialFlow(Keysmith *app)
    : QObject(app)
    , m_started(false)
    , m_uriToAdd(false)
    , m_uriParsed(false)
    , m_passwordPromptResolved(false)
    , m_app(app)
    , m_input(new model::AccountInput(this))
    , m_passwordRequest(new model::PasswordRequest(storageOf(app)->secret(), this))
{
    QObject::connect(m_passwordRequest, &model::PasswordRequest::passwordRequestChanged, this, &InitialFlow::resume);
    QObject::connect(m_passwordRequest, &model::PasswordRequest::passwordAccepted, this, &InitialFlow::resume);
    QObject::connect(accountListOf(m_app), &model::SimpleAccountListModel::loadedChanged, this, &InitialFlow::resume);
}

void InitialFlow::run(const QCommandLineParser &parser)
{
    flowStateOf(m_app)->setFlowRunning(true);
    overviewStateOf(m_app)->setActionsEnabled(false);
    m_started = true;

    const auto argv = parser.positionalArguments();
    if (argv.isEmpty()) {
        qCDebug(logger) << "No URIs to handle, moving on:" << this;
        QTimer::singleShot(0, this, &InitialFlow::resume);
        return;
    }

    qCDebug(logger) << "Will first parse given URI(s):" << this;

    m_uriToAdd = true;
    auto job = new CommandLineAccountJob(m_input);
    QObject::connect(job, &CommandLineAccountJob::newAccountProcessed, this, &InitialFlow::onNewAccountProcessed);
    QObject::connect(job, &CommandLineAccountJob::newAccountInvalid, this, &InitialFlow::onNewAccountInvalid);
    job->run(argv[0]);
}

void InitialFlow::onNewAccountProcessed(void)
{
    Q_ASSERT_X(m_started, Q_FUNC_INFO, "should have properly started the flow first");

    m_uriParsed = true;

    auto vm = new AddAccountViewModel(m_input, accountListOf(m_app), true, false);
    QObject::connect(vm, &AddAccountViewModel::accepted, this, &InitialFlow::resume);
    QObject::connect(vm, &AddAccountViewModel::cancelled, this, &InitialFlow::onNewAccountRejected);
    navigationFor(m_app)->navigate(Navigation::Page::AddAccount, vm);
}

void InitialFlow::onNewAccountAccepted(void)
{
    Q_ASSERT_X(m_started, Q_FUNC_INFO, "should have properly started the flow first");
    Q_ASSERT_X(m_uriToAdd && m_uriParsed, Q_FUNC_INFO, "should have parsed URIs first");

    accountListOf(m_app)->addAccount(m_input);
    m_uriToAdd = false;
    QTimer::singleShot(0, this, &InitialFlow::resume);
}

void InitialFlow::onNewAccountRejected(void)
{
    Q_ASSERT_X(m_uriToAdd && m_uriParsed, Q_FUNC_INFO, "should have parsed URIs first");
    m_uriToAdd = false;
    QTimer::singleShot(0, this, &InitialFlow::resume);
}

void InitialFlow::onNewAccountInvalid(void)
{
    Q_ASSERT_X(m_started, Q_FUNC_INFO, "should have properly started the flow first");

    auto vm = new ErrorViewModel(
        i18nc("@title:window", "Invalid account"),
        i18nc("@info:label", "The account you are trying to add is invalid. You can either quit the app, or continue without adding the account."),
        true);
    QObject::connect(vm, &ErrorViewModel::dismissed, this, &InitialFlow::onNewAccountRejected);
    navigationFor(m_app)->navigate(Navigation::Page::Error, vm);
}

void InitialFlow::resume(void)
{
    if (!m_started) {
        qCDebug(logger) << "Blocking progress: flow has not been started yet:" << this;
        return;
    }

    if (m_uriToAdd && !m_uriParsed) {
        qCDebug(logger) << "Blocking progress: URI parsing has not completed yet:" << this;
        return;
    }

    if (m_passwordRequest->keyAvailable()) {
        if (m_uriToAdd) {
            const auto accounts = accountListOf(m_app);
            if (!accounts->isAccountStillAvailable(m_input->name(), m_input->issuer())) {
                auto vm = new RenameAccountViewModel(m_input, accountListOf(m_app));
                QObject::connect(vm, &RenameAccountViewModel::cancelled, this, &InitialFlow::onNewAccountRejected);
                QObject::connect(vm, &RenameAccountViewModel::accepted, this, &InitialFlow::onNewAccountAccepted);
                navigationFor(m_app)->navigate(Navigation::Page::RenameAccount, vm);
                return;
            }
            if (accounts->loaded()) {
                QTimer::singleShot(0, this, &InitialFlow::onNewAccountAccepted);
                return;
            }

            qCDebug(logger) << "Blocking progress: accounts not fully loaded:"
                            << "Waiting to see if new account remains available:" << this;
            return;
        }

        auto vm = new AccountsOverviewViewModel(m_app);
        navigationFor(m_app)->navigate(Navigation::Page::AccountsOverview, vm);
        overviewStateOf(m_app)->setActionsEnabled(true);
        auto flows = flowStateOf(m_app);
        flows->setFlowRunning(false);
        flows->setInitialFlowDone(true);
        QTimer::singleShot(0, this, &QObject::deleteLater);
        return;
    }

    if (!m_passwordPromptResolved) {
        if (m_passwordRequest->firstRun()) {
            m_passwordPromptResolved = true;
            auto vm = new SetupPasswordViewModel(m_passwordRequest);
            navigationFor(m_app)->navigate(Navigation::Page::SetupPassword, vm);
            return;
        }
        if (m_passwordRequest->previouslyDefined()) {
            m_passwordPromptResolved = true;
            auto vm = new UnlockAccountsViewModel(m_passwordRequest);
            navigationFor(m_app)->navigate(Navigation::Page::UnlockAccounts, vm);
            return;
        }
        qCDebug(logger) << "Blocking progress: password request has not yet been resolved:" << this;
        return;
    }

    qCDebug(logger) << "Blocking progress: waiting for the next event:" << this;
}

ManualAddAccountFlow::ManualAddAccountFlow(Keysmith *app)
    : QObject(app)
    , m_app(app)
    , m_input(new model::AccountInput(this))
{
    Q_ASSERT_X(app, Q_FUNC_INFO, "should have a Keysmith instance");
}

void ManualAddAccountFlow::run(void)
{
    flowStateOf(m_app)->setFlowRunning(true);
    overviewStateOf(m_app)->setActionsEnabled(false);

    auto vm = new AddAccountViewModel(m_input, accountListOf(m_app), false, true);
    QObject::connect(vm, &AddAccountViewModel::accepted, this, &ManualAddAccountFlow::onAccepted);
    QObject::connect(vm, &AddAccountViewModel::cancelled, this, &ManualAddAccountFlow::back);
    navigationFor(m_app)->push(Navigation::Page::AddAccount, vm);
}

void ManualAddAccountFlow::onAccepted(void)
{
    accountListOf(m_app)->addAccount(m_input);
    QTimer::singleShot(0, this, &ManualAddAccountFlow::back);
}

void ManualAddAccountFlow::back(void)
{
    auto vm = new AccountsOverviewViewModel(m_app);
    navigationFor(m_app)->navigate(Navigation::Page::AccountsOverview, vm);
    overviewStateOf(m_app)->setActionsEnabled(true);
    flowStateOf(m_app)->setFlowRunning(false);
    QTimer::singleShot(0, this, &QObject::deleteLater);
}

ManualImportAccountFlow::ManualImportAccountFlow(Keysmith *app) :
    QObject(app), m_app(app), m_input(new model::ImportInput(this))
{
    Q_ASSERT_X(app, Q_FUNC_INFO, "should have a Keysmith instance");
}

void ManualImportAccountFlow::run(void)
{
    flowStateOf(m_app)->setFlowRunning(true);
    overviewStateOf(m_app)->setActionsEnabled(false);

    auto vm = new ImportAccountViewModel(m_input, accountListOf(m_app), false, true);
    QObject::connect(vm, &ImportAccountViewModel::accepted, this, &ManualImportAccountFlow::onAccepted);
    QObject::connect(vm, &ImportAccountViewModel::cancelled, this, &ManualImportAccountFlow::back);
    navigationFor(m_app)->push(Navigation::Page::ImportAccount, vm);
}

void ManualImportAccountFlow::onAccepted(void)
{
    for (model::AccountInput *input : m_input->importAccounts()) {
        accountListOf(m_app)->addAccount(input);
    }
    QTimer::singleShot(0, this, &ManualImportAccountFlow::back);
}

void ManualImportAccountFlow::back(void)
{
    auto vm = new AccountsOverviewViewModel(m_app);
    navigationFor(m_app)->navigate(Navigation::Page::AccountsOverview, vm);
    overviewStateOf(m_app)->setActionsEnabled(true);
    flowStateOf(m_app)->setFlowRunning(false);
    QTimer::singleShot(0, this, &QObject::deleteLater);
}

ExternalCommandLineFlow::ExternalCommandLineFlow(Keysmith *app)
    : QObject(app)
    , m_app(app)
    , m_input(new model::AccountInput(this))
{
    Q_ASSERT_X(app, Q_FUNC_INFO, "should have a Keysmith instance");
}

void ExternalCommandLineFlow::run(const QCommandLineParser &parser)
{
    const auto argv = parser.positionalArguments();
    if (argv.isEmpty()) {
        qCDebug(logger) << "No URIs to handle, nothing to do for external commandline:" << this;
        QTimer::singleShot(0, this, &ExternalCommandLineFlow::deleteLater);
        return;
    }

    flowStateOf(m_app)->setFlowRunning(true);
    overviewStateOf(m_app)->setActionsEnabled(false);
    qCDebug(logger) << "Will parse given URI(s) from external commandline:" << this;

    auto job = new CommandLineAccountJob(m_input);
    QObject::connect(job, &CommandLineAccountJob::newAccountProcessed, this, &ExternalCommandLineFlow::onNewAccountProcessed);
    QObject::connect(job, &CommandLineAccountJob::newAccountInvalid, this, &ExternalCommandLineFlow::onNewAccountInvalid);
    job->run(argv[0]);
}

void ExternalCommandLineFlow::onNewAccountProcessed(void)
{
    auto vm = new AddAccountViewModel(m_input, accountListOf(m_app), false, true);
    QObject::connect(vm, &AddAccountViewModel::accepted, this, &ExternalCommandLineFlow::onAccepted);
    QObject::connect(vm, &AddAccountViewModel::cancelled, this, &ExternalCommandLineFlow::back);
    navigationFor(m_app)->push(Navigation::Page::AddAccount, vm);
}

void ExternalCommandLineFlow::onNewAccountInvalid(void)
{
    auto vm = new ErrorViewModel(i18nc("@title:window", "Invalid account"),
                                 i18nc("@info:label", "The account you are trying to add is invalid. Continue without adding the account."),
                                 false);
    QObject::connect(vm, &ErrorViewModel::dismissed, this, &ExternalCommandLineFlow::back);
    navigationFor(m_app)->navigate(Navigation::Page::Error, vm);
}

void ExternalCommandLineFlow::onAccepted(void)
{
    accountListOf(m_app)->addAccount(m_input);
    QTimer::singleShot(0, this, &ExternalCommandLineFlow::back);
}

void ExternalCommandLineFlow::back(void)
{
    auto vm = new AccountsOverviewViewModel(m_app);
    navigationFor(m_app)->navigate(Navigation::Page::AccountsOverview, vm);
    overviewStateOf(m_app)->setActionsEnabled(true);
    flowStateOf(m_app)->setFlowRunning(false);
    QTimer::singleShot(0, this, &QObject::deleteLater);
}

AddAccountFromQRFlow::AddAccountFromQRFlow(Keysmith *app)
    : QObject(app)
    , m_app(app)
    , m_input(new model::AccountInput(this))
    , m_scan_vm(new ScanQRViewModel(this))
{
    Q_ASSERT_X(app, Q_FUNC_INFO, "should have a Keysmith instance");
}

void AddAccountFromQRFlow::run(void)
{
    flowStateOf(m_app)->setFlowRunning(true);
    overviewStateOf(m_app)->setActionsEnabled(false);

    QObject::connect(m_scan_vm, &ScanQRViewModel::scanComplete, this, &AddAccountFromQRFlow::scanComplete);
    QObject::connect(m_scan_vm, &ScanQRViewModel::scanCompleteText, this, &AddAccountFromQRFlow::scanCompleteText);
    QObject::connect(m_scan_vm, &ScanQRViewModel::cancelled, this, &AddAccountFromQRFlow::back);
    m_scan_vm->setActive(true);
    navigationFor(m_app)->push(Navigation::Page::ScanQR, m_scan_vm);
}

void AddAccountFromQRFlow::scanComplete(const QByteArray &uri)
{
    const auto result = model::QrParameters::parse(uri);

    if (!result) {
        m_scan_vm->setActive(true);
        return;
    }

    m_scanned = true;
    result->populate(m_input);
    parseComplete();
}

void AddAccountFromQRFlow::scanCompleteText(const QString &uri)
{
    const auto result = model::QrParameters::parse(uri);

    if (!result) {
        m_scan_vm->setActive(true);
        return;
    }

    m_scanned = true;
    result->populate(m_input);
    parseComplete();
}

void AddAccountFromQRFlow::parseComplete(void)
{
    auto vm = new AddAccountViewModel(m_input, accountListOf(m_app), false, true);
    QObject::connect(vm, &AddAccountViewModel::accepted, this, &AddAccountFromQRFlow::onAccepted);
    QObject::connect(vm, &AddAccountViewModel::cancelled, this, &AddAccountFromQRFlow::back);
    navigationFor(m_app)->push(Navigation::Page::AddAccount, vm);
}

void AddAccountFromQRFlow::onAccepted(void)
{
    accountListOf(m_app)->addAccount(m_input);
    QTimer::singleShot(0, this, &AddAccountFromQRFlow::exit);
}

void AddAccountFromQRFlow::back(void)
{
    if (m_scanned) {
        m_scanned = false;
        m_scan_vm->setActive(true);
        navigationFor(m_app)->pop();
    } else {
        exit();
    }
}

void AddAccountFromQRFlow::exit(void)
{
    auto vm = new AccountsOverviewViewModel(m_app);
    navigationFor(m_app)->navigate(Navigation::Page::AccountsOverview, vm);
    overviewStateOf(m_app)->setActionsEnabled(true);
    flowStateOf(m_app)->setFlowRunning(false);
    QTimer::singleShot(0, this, &QObject::deleteLater);
}
}

#include "moc_flows_p.cpp"
