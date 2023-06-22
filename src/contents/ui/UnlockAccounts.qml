/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import Keysmith.Application 1.0 as Application

Kirigami.ScrollablePage {
    id: root

    required property Application.UnlockAccountsViewModel vm

    leftPadding: 0
    rightPadding: 0

    title: i18nc("@title:window", "Password")

    header: QQC2.Control {
        padding: Kirigami.Units.smallSpacing
        contentItem: Kirigami.InlineMessage {
            id: errorMessage
            text: i18n("Failed to unlock your accounts")
            visible: vm.failed
            showCloseButton: true
            type: Kirigami.MessageType.Error
        }
    }

    Component.onCompleted: existingPassword.forceActiveFocus()

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
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Please provide the password to unlock your accounts")
                }

                MobileForm.FormTextFieldDelegate {
                    id: existingPassword
                    echoMode: TextInput.Password
                    label: i18nc("@label:textbox", "Password:")
                    enabled: !vm.busy
                    onAccepted: unlockAction.trigger()
                }

                MobileForm.FormDelegateSeparator { above: unlockButton }

                MobileForm.FormButtonDelegate {
                    id: unlockButton
                    action: Kirigami.Action {
                        id: unlockAction
                        text: i18n("Unlock")
                        iconName: "unlock"
                        enabled: !vm.busy && existingPassword.text && existingPassword.text.length > 0
                        visible: !Kirigami.Settings.isMobile
                        onTriggered: vm.unlock(existingPassword.text);
                    }
                }
            }
        }
    }
}
