/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models

Kirigami.ScrollablePage {
    id: root
    /*
     * Explicitly opt-out of scroll-to-refresh/drag-to-refresh behaviour
     * Underlying model implementations don't offer the hooks for that.
     */
    supportsRefreshing: false
    title: i18nc("@title:window", "Accounts")

    signal accountWanted
    property bool addActionEnabled : true
    property Models.AccountListModel accounts: Keysmith.accountListModel()

    Component {
        id: mainListDelegate
        AccountEntryView {
            account: model.account
        }
    }

    ListView {
        id: mainList
        model: accounts
        delegate: mainListDelegate
    }

    actions.main: Kirigami.Action {
        id: addAction
        text: i18n("Add")
        iconName: "list-add"
        visible: addActionEnabled
        onTriggered: {
            root.accountWanted();
        }
    }
}
