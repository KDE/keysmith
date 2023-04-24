/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.5 as Controls
import org.kde.kirigami 2.8 as Kirigami

import Keysmith.Application 1.0 as Application

Kirigami.ScrollablePage {
    id: root
    title: i18nc("@title:window", "Password")

    property Application.SetupPasswordViewModel vm

    Component.onCompleted: newPassword.forceActiveFocus()

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        
        Kirigami.Icon {
            source: "lock"
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 10
            Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            Layout.preferredHeight: width
            Layout.leftMargin: Kirigami.Units.gridUnit * 3
            Layout.rightMargin: Kirigami.Units.gridUnit * 3
        }
        
        Kirigami.Heading {
            level: 3
            text: i18n("Choose a password to protect your accounts")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            horizontalAlignment: form.wideMode ? Qt.AlignHCenter : Qt.AlignLeft
            verticalAlignment: Qt.AlignTop
        }
        
        Kirigami.FormLayout {
            id: form
            Layout.fillWidth: true
            Kirigami.PasswordField {
                id: newPassword
                text: ""
                enabled: !vm.busy
                Kirigami.FormData.label: i18nc("@label:textbox", "New password:")
                onAccepted: newPasswordCopy.forceActiveFocus()
            }
            Kirigami.PasswordField {
                id: newPasswordCopy
                text: ""
                enabled: !vm.busy
                Kirigami.FormData.label: i18nc("@label:textbox", "Verify password:")
                onAccepted: applyAction.trigger()
            }
        }
    }

    actions.main : Kirigami.Action {
        id: applyAction
        text: i18n("Apply")
        iconName: "answer-correct"
        enabled: !vm.busy && newPassword.text === newPasswordCopy.text && newPassword.text && newPassword.text.length > 0
        onTriggered: {
            vm.setup(newPassword.text, newPasswordCopy.text);
        }
    }

    footer: Controls.Control {
        padding: Kirigami.Units.smallSpacing
        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            text: i18n("Failed to set up your password")
            visible: vm.failed
            showCloseButton: true
        }
    }
}
