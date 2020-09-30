/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    property bool addActionEnabled: !addAccountRequested
    property bool addAccountAvailable: false
    property bool addAccountConfirmed: false
    property bool addAccountRequested: CommandLine.newAccountRequested
    property Models.AccountListModel accounts: Keysmith.accountListModel()
    property Models.ValidatedAccountInput validatedInput: Models.ValidatedAccountInput {}
    property Models.PasswordRequestModel passwordRequest: Keysmith.passwordRequest()

    property Component passwordRequestPage: passwordRequest.previouslyDefined ? unlockAccountsPage : setupPasswordPage

    Component {
        id: setupPasswordPage
        SetupPassword {
        }
    }

    Component {
        id: unlockAccountsPage
        UnlockAccounts {
        }
    }

    Component {
        id: accountsOverviewPage
        AccountsOverview {
            accounts: root.accounts
            addActionEnabled: root.addActionEnabled
            onAccountWanted: {
                pageStack.push(addPageComponent);
                root.addActionEnabled = false;
            }
        }
    }

    Component {
        id: addFromCommandLinePageComponent
        AddAccount {
            quitEnabled: true
            id: addAccountFromCommandLinePage
            validateAccountAvailability: false
            validatedInput: root.validatedInput
            accounts: root.accounts
            onQuit: {
                pageStack.pop();
                Qt.quit();
            }
            onCancelled: {
                root.addAccountConfirmed = false;
                popAddAccountPage(passwordRequestPage);
            }
            onNewAccount: {
                root.addAccountConfirmed = true;
                popAddAccountPage(passwordRequestPage);
            }
        }
    }

    Component {
        id: renameFromCommandLinePageComponent
        RenameAccount {
            id: renameFromCommandLinePage
            accounts: root.accounts
            validatedInput: root.validatedInput
            onCancelled: {
                popAddAccountPage(accountsOverviewPage);
            }
            onNewAccount: {
                root.accounts.addAccount(root.validatedInput);
                popAddAccountPage(accountsOverviewPage);
            }
        }
    }

    Component {
        id: invalidUriFromCommandLineErrorPage
        ErrorPage {
            quitEnabled: true
            title: i18nc("@title:window", "Invalid account")
            error: i18nc("@info:label", "The account you are trying to add is invalid. You can either quit the app, or continue without adding the account.")
            onDismissed: {
                popAddAccountPage(passwordRequestPage);
            }
            onQuit: {
                pageStack.pop();
                Qt.quit();
            }
        }
    }

    function autoAddNewAccountFromCommandLine() {
        // accounts not yet loaded, keep the user waiting...
        if (!accounts.loaded) {
            return;
        }

        if (!passwordRequest.keyAvailable) {
            return; // TODO warn if not
        }

        // nothing to do (anymore)
        if (!root.addAccountConfirmed) {
            return;
        }

        // claim the new account, try to see if it can be added directly or needs renaming/recovery
        addAccountConfirmed = false;
        if(accounts.isAccountStillAvailable(validatedInput.name, validatedInput.issuer)) {
            accounts.addAccount(validatedInput);
            pageStack.push(accountsOverviewPage);
        } else {
            pageStack.push(renameFromCommandLinePageComponent);
        }
    }

    Component {
        id: addPageComponent
        AddAccount {
            id: addAccountPage
            accounts: root.accounts
            onCancelled: {
                popAddAccountPage();
            }
            onNewAccount: {
                popAddAccountPage();
                root.accounts.addAccount(input);
            }
        }
    }

    function popAddAccountPage(next) {
        pageStack.pop();
        addActionEnabled = true;
        if (next) {
            pageStack.push(next);
        }
    }

    /*
     * TODO maybe have a onPasswordProvided handler to push a "progress" page to provide visual feedback for devices
     * where key derivation is slow?
     */
    Connections {
        target: passwordRequest
        onDerivedKey : {
            // TODO convert to C++ helper, have proper logging?
            if (!passwordRequest.keyAvailable) {
                return; // TODO warn if not
            }

            pageStack.pop();
            if (root.addAccountConfirmed) {
                autoAddNewAccountFromCommandLine();
            } else {
                pageStack.push(accountsOverviewPage);
            }
        }
        onPreviouslyDefinedChanged: {
            /*
             * Ignore if there is an account from the commandline to process: in that case password unlocking/setup is
             * deferred until after account confirmation/rejection by the user
             */
            if (!addAccountRequested) {
                /*
                 * Cannot rely on passwordRequestPage property binding having been updated already here, work around
                 * by repeating the passwordRequestPage property binding expression instead.
                 */
                pageStack.push(passwordRequest.previouslyDefined ? unlockAccountsPage : setupPasswordPage);
            }
        }
    }

    Connections {
        target: accounts
        onLoadedChanged: {
            if (accounts.loaded) {
                autoAddNewAccountFromCommandLine();
            }
        }
    }

    Connections {
        target: CommandLine

        onNewAccountInvalid: {
            // TODO properly report error in UX

            if (addAccountRequested) {
                addAccountAvailable = false;
                addAccountConfirmed = false;
                addActionEnabled = true;
                pageStack.push(invalidUriFromCommandLineErrorPage);
            }
            // TODO warn if not
        }

        onNewAccountProcessed: {
            addAccountAvailable = true;
            pageStack.push(addFromCommandLinePageComponent);
        }
    }

    Component.onCompleted: {
        if (addAccountRequested) {
            root.validatedInput.reset();
            CommandLine.handleNewAccount(root.validatedInput);
        }
    }
}
