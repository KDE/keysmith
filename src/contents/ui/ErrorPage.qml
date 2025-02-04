/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

import Keysmith.Application 1.0 as Application

Kirigami.Page {
    id: root

    property Application.ErrorViewModel vm

    title: vm.errorTitle

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        Controls.Label {
            text: vm.errorText
            color: Kirigami.Theme.negativeTextColor
            Layout.maximumWidth: root.width - 2 * Kirigami.Units.largeSpacing
            wrapMode: Text.WordWrap
        }
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button Button to dismiss the error page", "Continue")
            icon.name: "answer-correct"
            onTriggered: {
                vm.dismissed();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button Dismiss the error page and quit Keysmtih", "Quit")
            icon.name: "application-exit"
            enabled: vm.quitEnabled
            visible: vm.quitEnabled
            onTriggered: {
                Qt.quit();
            }
        }
    ]
}
