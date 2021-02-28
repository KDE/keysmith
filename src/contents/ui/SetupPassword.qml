/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.5 as Controls
import org.kde.kirigami 2.8 as Kirigami

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models

Kirigami.ScrollablePage {
    id: root
    title: i18nc("@title:window", "Password")

    property bool bannerTextError : false
    property Models.PasswordRequestModel passwordRequest: Keysmith.passwordRequest()

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
        }
        
        Kirigami.FormLayout {
            id: form
            Layout.fillWidth: true
            Kirigami.PasswordField {
                id: newPassword
                text: ""
                enabled: !passwordRequest.passwordProvided
                Kirigami.FormData.label: i18nc("@label:textbox", "New password:")
                onAccepted: newPasswordCopy.forceActiveFocus()
            }
            Kirigami.PasswordField {
                id: newPasswordCopy
                text: ""
                enabled: !passwordRequest.passwordProvided
                Kirigami.FormData.label: i18nc("@label:textbox", "Verify password:")
                onAccepted: applyAction.trigger()
            }
        }
        
        Connections {
            target: passwordRequest
            onPasswordRejected: {
                bannerTextError = true
            }
        }
    }
    

    actions.main : Kirigami.Action {
        id: applyAction
        text: i18n("Apply")
        iconName: "answer-correct"
        enabled: !passwordRequest.passwordProvided && newPassword.text === newPasswordCopy.text && newPassword.text && newPassword.text.length > 0
        onTriggered: {
            // TODO convert to C++ helper, have proper logging?
            if (passwordRequest) {
                bannerTextError = !passwordRequest.provideBothPasswords(newPassword.text, newPasswordCopy.text);
            }
            // TODO warn if not
        }
    }

    footer: Controls.Control {
        padding: Kirigami.Units.smallSpacing
        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            text: i18n("Failed to set up your password")
            visible: bannerTextError
            showCloseButton: true
        }
    }
}
