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

    pageStack.initialPage: accountsOverviewPage

    property bool addActionEnabled: true
    property Models.AccountListModel accounts: Keysmith.accountListModel()

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
}
