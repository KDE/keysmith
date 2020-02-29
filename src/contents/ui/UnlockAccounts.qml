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

Kirigami.Page {
    id: root
    title: i18nc("@title:window", "Password")

    property bool bannerTextError : false
    property string bannerText : i18n("Please provide the password to unlock your accounts")
    property string failedToApplyPassword : i18n("Failed to unlock your accounts")
    property Models.PasswordRequestModel passwordRequest: Keysmith.passwordRequest()

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        Controls.Label {
            text: bannerText
            color: bannerTextError ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        }
        Kirigami.FormLayout {
            Kirigami.PasswordField {
                id: existingPassword
                text: ""
                Kirigami.FormData.label: i18nc("@label:textbox", "Password:")
                inputMethodHints:  Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
            }
        }
    }

    actions.main : Kirigami.Action {
        text: i18n("Unlock")
        iconName: "unlock"
        enabled: existingPassword.text && existingPassword.text.length > 0
        onTriggered: {
            // TODO convert to C++ helper, have proper logging?
            if (passwordRequest) {
                if (!passwordRequest.providePassword(existingPassword.text)) {
                    bannerText = failedToApplyPassword;
                    bannerTextError = true;
                }
            }
            // TODO warn if not
        }
    }
}
