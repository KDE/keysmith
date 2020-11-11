/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
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

    // HACK remove when depends on Kirigami 5.77
    Component.onCompleted: {
        for (var index in form.children[0].children) {
            var item = form.children[0].children[index];
            if (item instanceof Text) {
                item.wrapMode = item.Text.Wrap
            }
            item.Layout.fillWidth = true;
        }
    }

    Kirigami.FormLayout {
        id: form
        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Get started by choosing a password to protect your accounts")
        }
        Kirigami.PasswordField {
            id: newPassword
            text: ""
            Kirigami.FormData.label: i18nc("@label:textbox", "New password:")
            onAccepted: newPasswordCopy.forceActiveFocus()
        }
        Kirigami.PasswordField {
            id: newPasswordCopy
            text: ""
            Kirigami.FormData.label: i18nc("@label:textbox", "Verify password:")
            onAccepted: applyAction.trigger()
        }
    }

    actions.main : Kirigami.Action {
        id: applyAction
        text: i18n("Apply")
        iconName: "answer-correct"
        enabled: newPassword.text === newPasswordCopy.text && newPassword.text && newPassword.text.length > 0
        onTriggered: {
            // TODO convert to C++ helper, have proper logging?
            if (passwordRequest) {
                if (!passwordRequest.provideBothPasswords(newPassword.text, newPasswordCopy.text)) {
                    bannerTextError = true;
                }
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
