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
    property string bannerText : i18n("Get started by choosing a password to protect your accounts")
    property string failedToApplyNewPassword : i18n("Failed to set up your password")
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
                id: newPassword
                text: ""
                Kirigami.FormData.label: i18nc("@label:textbox", "New password:")
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
            }
            Kirigami.PasswordField {
                id: newPasswordCopy
                text: ""
                Kirigami.FormData.label: i18nc("@label:textbox", "Verify password:")
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
            }
        }
    }

    actions.main : Kirigami.Action {
        text: i18n("Apply")
        iconName: "answer-correct"
        enabled: newPassword.text === newPasswordCopy.text && newPassword.text && newPassword.text.length > 0
        onTriggered: {
            // TODO convert to C++ helper, have proper logging?
            if (passwordRequest) {
                if (!passwordRequest.provideBothPasswords(newPassword.text, newPasswordCopy.text)) {
                    bannerText = failedToApplyNewPassword;
                    bannerTextError = true;
                }
            }
            // TODO warn if not
        }
    }
}
