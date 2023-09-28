/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kirigamiaddons.components 1.0 as Components

import Keysmith.Application as Application

FormCard.FormCardPage {
    id: root

    required property Application.SetupPasswordViewModel vm

    title: i18nc("@title:window", "Password")

    Component.onCompleted: newPassword.forceActiveFocus()

    header: Components.Banner {
        id: errorMessage
        text: i18n("Failed to set up your password")
        visible: vm.failed
        showCloseButton: true
    }

    Kirigami.Icon {
        source: "lock"
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 8
        Layout.topMargin: Kirigami.Units.gridUnit
        Layout.bottomMargin: Kirigami.Units.gridUnit
    }

    FormCard.FormHeader {
        title: i18n("Choose a password to protect your accounts")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: newPassword
            echoMode: TextInput.Password
            label: i18nc("@label:textbox", "New Password:")
            enabled: !vm.busy
            onAccepted: newPasswordCopy.trigger()
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: newPasswordCopy
            enabled: !vm.busy
            label: i18nc("@label:textbox", "Verify password:")
            onAccepted: applyButton.clicked()
        }

        FormCard.FormDelegateSeparator { above: applyButton }

        FormCard.FormButtonDelegate {
            id: applyButton
            text: i18nc("@action:button", "Apply")
            icon.name: "answer-correct"
            enabled: !vm.busy && newPassword.text === newPasswordCopy.text && newPassword.text && newPassword.text.length > 0
            onClicked: vm.setup(newPassword.text, newPasswordCopy.text);
        }
    }
}
