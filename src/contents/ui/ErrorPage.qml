/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

Kirigami.Page {
    id: root
    signal quit
    signal dismissed
    property bool quitEnabled: false
    property string error

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        Controls.Label {
            text: root.error
            color: Kirigami.Theme.negativeTextColor
            Layout.maximumWidth: root.width - 2 * Kirigami.Units.largeSpacing
            wrapMode: Text.WordWrap
        }
    }

    actions.main: Kirigami.Action {
        text: i18nc("@action:button Button to dismiss the error page", "Continue")
        iconName: "answer-correct"
        onTriggered: {
            root.dismissed();
        }
    }
    actions.right: Kirigami.Action {
        text: i18nc("@action:button Dismiss the error page and quit Keysmtih", "Quit")
        iconName: "application-exit"
        enabled: root.quitEnabled
        visible: root.quitEnabled
        onTriggered: {
            root.quit();
        }
    }
}
