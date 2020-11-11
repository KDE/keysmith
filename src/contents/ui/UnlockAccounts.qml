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
    title: i18nc("@title:window", "Password")

    property bool bannerTextError : false
    property Models.PasswordRequestModel passwordRequest: Keysmith.passwordRequest()

    header: Kirigami.InlineMessage {
        id: errorMessage
        Layout.fillWidth: true
        text: i18n("Failed to unlock your accounts")
        visible: bannerTextError
        showCloseButton: true
        type: Kirigami.MessageType.Error
    }
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
            Kirigami.FormData.label: i18n("Please provide the password to unlock your accounts")
        }
        Kirigami.PasswordField {
            id: existingPassword
            text: ""
            Kirigami.FormData.label: i18nc("@label:textbox", "Password:")
            onAccepted: {
                if (unlockAction.enabled) {
                    unlockAction.trigger()
                }
            }
        }
    }

    actions.main : Kirigami.Action {
        id: unlockAction
        text: i18n("Unlock")
        iconName: "unlock"
        enabled: existingPassword.text && existingPassword.text.length > 0
        onTriggered: {
            // TODO convert to C++ helper, have proper logging?
            if (passwordRequest) {
                if (!passwordRequest.providePassword(existingPassword.text)) {
                    bannerTextError = true;
                }
            }
            // TODO warn if not
        }
    }
}
