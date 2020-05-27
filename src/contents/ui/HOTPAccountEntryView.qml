/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import org.kde.kirigami 2.4 as Kirigami

AccountEntryViewBase {
    id: root

    actions: [
        Kirigami.Action {
            iconName: "edit-delete"
            text: i18nc("Button for removal of a single account", "Delete account")
            enabled: root.alive
            onTriggered: {
                root.actionTriggered();
                root.sheet.open();
            }
        },
        Kirigami.Action {
            iconName: "go-next" // "view-refresh"
            text: "Next token"
            enabled: root.alive
            onTriggered: {
                root.actionTriggered();
                root.account.advanceCounter(1);
            }
        }
    ]

    TokenEntryViewLabels {
        id: mainLayout
        accountName: account.name
        tokenValue: account.token
        labelColor: root.labelColor
    }
}
