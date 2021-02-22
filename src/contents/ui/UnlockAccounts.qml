/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

import Keysmith.Application 1.0 as Application

Kirigami.ScrollablePage {
    id: root
    title: i18nc("@title:window", "Password")

    property Application.UnlockAccountsViewModel vm

    header: Controls.Control {
        padding: Kirigami.Units.smallSpacing
        contentItem: Kirigami.InlineMessage {
            id: errorMessage
            text: i18n("Failed to unlock your accounts")
            visible: vm.failed
            showCloseButton: true
            type: Kirigami.MessageType.Error
        }
    }

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
            text: i18n("Please provide the password to unlock your accounts")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            horizontalAlignment: form.wideMode ? Qt.AlignHCenter : Qt.AlignLeft
        }
        
        Kirigami.FormLayout {
            id: form
            Layout.fillWidth: true
            Kirigami.PasswordField {
                id: existingPassword
                text: ""
                Kirigami.FormData.label: i18nc("@label:textbox", "Password:")
                enabled: !vm.busy
                onAccepted: {
                    if (unlockAction.enabled) {
                        unlockAction.trigger()
                    }
                }
            }
        }
    }

    actions.main : Kirigami.Action {
        id: unlockAction
        text: i18n("Unlock")
        iconName: "unlock"
        enabled: !vm.busy && existingPassword.text && existingPassword.text.length > 0
        onTriggered: {
            vm.unlock(existingPassword.text);
        }
    }
}
