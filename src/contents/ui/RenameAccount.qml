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
import Keysmith.Validators 1.0 as Validators

Kirigami.Page {
    id: root
    title: i18nc("@title:window", "Rename account to add")
    signal cancelled
    signal newAccount(var input)
    property Models.AccountListModel accounts

    property bool acceptable: accountName.acceptable
    property Models.ValidatedAccountInput validatedInput

    Connections {
        target: validatedInput
        onTypeChanged: {
            root.detailsEnabled = false;
        }
    }

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        Controls.Label {
            text:i18nc("@info:label Keysmith received an account to add via URI on e.g. commandline which is already in use", "Another account with the same name already exists. Please correct the name or issuer for the new account.")
            color: Kirigami.Theme.negativeTextColor
            Layout.maximumWidth: root.width - 2 * Kirigami.Units.largeSpacing
            wrapMode: Text.WordWrap
        }
        AccountNameForm {
            id: accountName
            accounts: root.accounts
            validateAccountAvailability: true
            validatedInput: root.validatedInput
        }
    }

    actions.left: Kirigami.Action {
        text: i18nc("@action:button cancel and dismiss the rename account form", "Cancel")
        iconName: "edit-undo"
        onTriggered: {
            root.cancelled();
        }
    }
    actions.main: Kirigami.Action {
        text: i18n("Add")
        iconName: "answer-correct"
        enabled: acceptable
        onTriggered: {
            root.newAccount(root.validatedInput);
        }
    }
}
