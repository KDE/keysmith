/*
 * Copyright 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License or any later version accepted by the membership of
 * KDE e.V. (or its successor approved by the membership of KDE
 * e.V.), which shall act as a proxy defined in Section 14 of
 * version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import Oath 1.0
import Oath.Validators 1.0 as Validators
import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.Page {
    id: root

    property Account account
    property int accountIndex;

    signal accountUpdate(Account account, int index)
    signal tokenRefresh(Account account, int index)

    property bool editMode: false
    property bool hideSensitive: true

    Kirigami.Action {
        id: leftAction
        text: root.hideSensitive ? i18n("Show") : i18n("Hide")
        iconName: root.hideSensitive ? "view-visible" : "view-hidden"
        onTriggered: {
            root.hideSensitive = !root.hideSensitive;
            root.editMode = false;
        }
    }

    Kirigami.Action {
        id: rightAction
        text: i18nc("@action:button", "Generate Token")
        iconName: "view-refresh"
        onTriggered: {
            root.tokenRefresh(account, accountIndex)
        }
    }

    Kirigami.Action {
        id: mainAction
        text: root.editMode ? i18n("Apply") : i18n("Edit")
        iconName: root.editMode ? "document-save" : "document-edit"
        onTriggered: {
            var fromEditor = root.editMode;
            root.editMode = !fromEditor;
            if (fromEditor) {
                accountUpdate(root.account, root.accountIndex);
            }
        }
    }

    actions.main: mainAction
    actions.left: editMode ? null : leftAction
    actions.right: editMode ? null : rightAction
    title: account ? account.name : i18nc("@title:window", "Account Details")

    ColumnLayout {
        id: layout
        TokenDetailsForm {
            id: tokenDetails
            account: root.account
            editable: editMode
        }
    }
}
