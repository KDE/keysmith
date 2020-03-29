/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.2 as Controls
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

    property string accountErrorMessage: i18nc("generic error shown when adding or updating an account failed", "Failed to update accounts")
    property string loadingErrorMessage: i18nc("error message shown when loading accounts from storage failed", "Some accounts failed to load.")
    property string errorMessage: loadingErrorMessage

    Component {
        id: mainListDelegate
        AccountEntryView {
            id: entry
            account: model.account
            onActionTriggered: {
                root.accounts.error = false;
                root.errorMessage = root.accountErrorMessage;
            }
        }
    }

    header: ColumnLayout {
        id: column
        Layout.margins: 0
        spacing: 0
        Kirigami.InlineMessage {
            id: message
            visible: root.accounts.error
            type: Kirigami.MessageType.Error
            text: root.errorMessage
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            /*
             * There is supposed to be a more Kirigami-way to allow the user to dismiss the error message: showCloseButton
             * Unfortunately:
             *
             *  - Kirigami doesn't really offer a direct API for detecting when the close button is clicked.
             *    Observing the close button's effect via the visible property works just as well, but it is a bit of a hack.
             *  - It results in a rather unattractive vertical sizing problem: the close button is rather big for inline text
             *    This makes the internal horizontal spacing look completely out of proportion with the vertical spacing.
             *  - The actual click/tap target is only a small fraction of the entire message (banner).
             *    In this case, making the entire message click/tap target would be much better.
             *
             * Solution: add a MouseArea for dismissing the message via click/tap.
             */
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.accounts.error = false;
                }
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: root.accounts.error
        }
    }

    ListView {
        id: mainList
        model: accounts
        Layout.fillWidth: true
        delegate: mainListDelegate
    }

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0
    actions.main: Kirigami.Action {
        id: addAction
        text: i18n("Add")
        iconName: "list-add"
        visible: addActionEnabled
        onTriggered: {
            root.accounts.error = false;
            root.errorMessage = root.accountErrorMessage;
            root.accountWanted();
        }
    }
}
