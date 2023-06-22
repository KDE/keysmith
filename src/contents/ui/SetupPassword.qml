/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import Keysmith.Application 1.0 as Application

Kirigami.ScrollablePage {
    id: root

    required property Application.SetupPasswordViewModel vm

    title: i18nc("@title:window", "Password")

    leftPadding: 0
    rightPadding: 0

    Component.onCompleted: newPassword.forceActiveFocus()

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            source: "lock"
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 8
        }

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Choose a password to protect your accounts")
                }

                MobileForm.FormTextFieldDelegate {
                    id: newPassword
                    echoMode: TextInput.Password
                    label: i18nc("@label:textbox", "New Password:")
                    enabled: !vm.busy
                    onAccepted: newPasswordCopy.trigger()
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormTextFieldDelegate {
                    id: newPasswordCopy
                    enabled: !vm.busy
                    label: i18nc("@label:textbox", "Verify password:")
                    onAccepted: applyAction.trigger()
                }

                MobileForm.FormDelegateSeparator { above: applyButton }

                MobileForm.FormButtonDelegate {
                    id: applyButton
                    action: Kirigami.Action {
                        id: applyAction
                        text: i18n("Apply")
                        iconName: "answer-correct"
                        enabled: !vm.busy && newPassword.text === newPasswordCopy.text && newPassword.text && newPassword.text.length > 0
                        onTriggered: vm.setup(newPassword.text, newPasswordCopy.text);
                    }
                }
            }
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
