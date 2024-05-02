/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models

Kirigami.SwipeListItem {
    /*
     * WARNING: AccountEntryViewBase is a derivative of SwipeListItem and SwipeListem instances *must* be
     * called `listItem`. This took *way* too long to figure out. If you change it, things will break for example the
     * flood fill effect when pressing a list entry on Android.
     */
    id: listItem

    signal actionTriggered
    property Models.Account account: null
    property bool alive: account !== null
    property bool tokenAvailable: alive && account.token && account.token.length > 0
    property bool highlightActive: listItem.pressed || listItem.checked
    property color labelColor: highlightActive ? listItem.activeTextColor : listItem.textColor

    visible: alive
    enabled: alive

    readonly property Component details: FormCard.FormCardPage {
        title: i18nc("Details dialog title: %1 is the name of the account", "Details of account: %1", account.name)

        topPadding: Kirigami.Units.gridUnit
        bottomPadding: Kirigami.Units.gridUnit

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Name:")
                description: account.name
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Issuer:")
                description: account.issuer
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Mode:")
                description: account.isHotp ? i18nc("Mode of 2fa", "HOTP") : i18nc("Mode of 2fa", "TOTP")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Epoch:")
                description: account.epoch
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Time Step:")
                description: account.timeStep
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Offset:")
                description: account.offset
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Token Length:")
                description: account.tokenLength
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Hash:")
                description: account.hash
            }
        }
    }

    readonly property Components.MessageDialog sheet: Components.MessageDialog {
        dialogType: Components.MessageDialog.Warning

        title: i18nc("Confirm dialog title: %1 is the name of the account to remove", "Removing account: %1", account.name)

        Controls.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("<p><ul><li>Account name: %1</li><li>Account issuer: %2</li></ul></p><p>Removing this account from Keysmith will not disable two-factor authentication (2FA). Make sure you can still access your account without using Keysmith before proceeding:</p><ul><li>Make sure you have another 2FA app set up for your account, or:</li><li>Make sure you have recovery codes for your account, or:</li><li>Disable two-factor authentication on your account</li></ul>", account.name, account.issuer)
        }

        footer: Controls.DialogButtonBox {
            padding: Kirigami.Units.largeSpacing * 2
            standardButtons: Controls.Dialog.Cancel

            onRejected: sheet.close();

            Controls.Button {
                icon.name: "edit-delete"
                text: i18nc("Button confirming account removal", "Delete Account")
                enabled: alive
                onClicked: {
                    alive = false;
                    account.remove();
                    sheet.close();
                }

                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
        }
    }

    onClicked: {
        // TODO convert to C++ helper, have proper logging?
        actionTriggered();
        if (tokenAvailable) {
            Keysmith.copyToClipboard(account.token);
            applicationWindow().showPassiveNotification(i18nc("Notification shown in a passive notification", "Token copied to clipboard!"))
        }
        // TODO warn if not
    }
}
