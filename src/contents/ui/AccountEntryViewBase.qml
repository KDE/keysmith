/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models

Kirigami.SwipeListItem {
    id: root

    signal actionTriggered
    property Models.Account account: null
    property bool alive: account !== null
    property bool tokenAvailable: alive && account.token && account.token.length > 0
    property color labelColor: root.pressed || root.checked ? root.activeTextColor : root.textColor

    visible: alive
    enabled: alive

    readonly property Kirigami.OverlaySheet sheet: Kirigami.OverlaySheet {
        sheetOpen: false
        header: Kirigami.Heading {
            text: i18nc("Confirm dialog title: %1 is the name of the account to remove", "Removing account: %1", account.name)
        }
        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing * 5
            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: i18n("<p>Removing this account from Keysmith will not disable two-factor authentication (2FA). Make sure you can still access your account without using Keysmith before proceeding:</p><ul><li>Make sure you have another 2FA app set up for your account, or:</li><li>Make sure you have recovery codes for your account, or:</li><li>Disable two-factor authentication on your account</li></ul>")
            }
        }
        footer: RowLayout {
            Controls.Button {
                action: Kirigami.Action {
                    iconName: "edit-undo"
                    text: i18nc("Button cancelling account removal", "Cancel")
                    onTriggered: {
                        sheet.close();
                    }
                }
            }
            Rectangle {
                color: "transparent"
                Layout.fillWidth: true
            }
            Controls.Button {
                action: Kirigami.Action {
                    iconName: "edit-delete"
                    text: i18nc("Button confirming account removal", "Delete account")
                    enabled: alive
                    onTriggered: {
                        alive = false;
                        account.remove();
                        sheet.close();
                    }
                }
            }
        }
    }

    onClicked: {
        // TODO convert to C++ helper, have proper logging?
        actionTriggered();
        if (tokenAvailable) {
            Keysmith.copyToClipboard(account.token);
        }
        // TODO warn if not
    }
}
