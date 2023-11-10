/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import org.kde.kirigami as Kirigami

AccountEntryViewBase {
    /*
     * WARNING: AccountEntryViewBase is a derivative of SwipeListItem and SwipeListem instances *must* be
     * called `listItem`. This took *way* too long to figure out. If you change it, things will break for example the
     * flood fill effect when pressing a list entry on Android.
     */
    id: listItem

    actions: [
        Kirigami.Action {
            icon.name: "documentinfo"
            text: i18nc("Button to show details of a single account", "Show details")
            enabled: listItem.alive
            onTriggered: {
                listItem.actionTriggered();
                applicationWindow().pageStack.pushDialogLayer(listItem.details);
            }
        },
        Kirigami.Action {
            icon.name: "edit-delete"
            text: i18nc("Button for removal of a single account", "Delete account")
            enabled: listItem.alive
            onTriggered: {
                listItem.actionTriggered();
                listItem.sheet.open();
            }
        },
        Kirigami.Action {
            icon.name: "go-next" // "view-refresh"
            text: "Next token"
            enabled: listItem.alive
            onTriggered: {
                listItem.actionTriggered();
                listItem.account.advanceCounter(1);
            }
        }
    ]

    contentItem: TokenEntryViewLabels {
        accountName: account.name
        tokenValue: account.token
        labelColor: listItem.labelColor
    }
}
