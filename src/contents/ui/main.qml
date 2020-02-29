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

    property bool addActionEnabled: true
    property Models.AccountListModel accounts: Keysmith.accountListModel()
    property Models.PasswordRequestModel passwordRequest: Keysmith.passwordRequest()

    pageStack.initialPage: passwordRequest.previouslyDefined ? unlockAccountsPage : setupPasswordPage

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
        id: addPageComponent
        AddAccount {
            accounts: root.accounts
            onDismissed: {
                pageStack.pop();
                addActionEnabled = true;
            }
        }
    }

    // TODO maybe have a onPasswordProvided handler to push a "progress" page to provide visual feedback for devices where key derivation is slow?
    Connections {
        target: passwordRequest
        onDerivedKey : {
            // TODO convert to C++ helper, have proper logging?
            if (passwordRequest.keyAvailable) {
                pageStack.pop();
                pageStack.push(accountsOverviewPage);
            }
            // TODO warn if not
        }
    }
}
