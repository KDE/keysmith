/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1 as FormCard

import Keysmith.Application as Application

FormCard.FormCardPage {
    id: root

    required property Application.UnlockAccountsViewModel vm

    title: i18nc("@title:window", "Password")

    header: Kirigami.InlineMessage {
        id: errorMessage
        text: i18n("Failed to unlock your accounts")
        visible: vm.failed
        showCloseButton: true
        type: Kirigami.MessageType.Error
        position: Kirigami.InlineMessage.Position.Header
    }

    Component.onCompleted: existingPassword.forceActiveFocus()

    Kirigami.Icon {
        source: "lock"
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 8
        Layout.topMargin: Kirigami.Units.gridUnit
        Layout.bottomMargin: Kirigami.Units.gridUnit
    }

    FormCard.FormHeader {
        title: i18n("Please provide the password to unlock your accounts")
    }

    FormCard.FormCard {
        FormCard.FormPasswordFieldDelegate {
            id: existingPassword
            label: i18nc("@label:textbox", "Password:")
            enabled: !vm.busy
            onAccepted: unlockButton.clicked()
        }

        FormCard.FormDelegateSeparator {
            below: existingPassword
            above: unlockButton
        }

        FormCard.FormButtonDelegate {
            id: unlockButton
            text: i18nc("@action:button", "Unlock")
            icon.name: "unlock"
            enabled: !vm.busy && existingPassword.text && existingPassword.text.length > 0
            onClicked: vm.unlock(existingPassword.text);
        }
    }
}
