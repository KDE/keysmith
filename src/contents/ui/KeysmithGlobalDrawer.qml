/*
* SPDX-License-Identifier: GPL-3.0-or-later
* SPDX-FileCopyrightText: 2024 Plata Hill <plata.hill@kdemail.net>
*/

import org.kde.kirigami as Kirigami

Kirigami.GlobalDrawer {
    isMenu: true
    actions: [
        Kirigami.Action {
            text: i18n("About Keysmith")
            icon.name: "org.kde.keysmith"
            onTriggered: pageStack.layers.push(Qt.createComponent('org.kde.kirigamiaddons.formcard', 'AboutPage'))
            enabled: pageStack.layers.currentItem.title !== i18n("About Keysmith")
        }
    ]
}
