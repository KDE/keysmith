/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Controls 2.0 as Controls

Item {
    property string accountName
    property string tokenValue

    id: root
    Controls.Label {
        id: accountNameLabel
        horizontalAlignment: Text.AlignLeft
        font.weight: Font.Light
        elide: Text.ElideRight
        text: accountName
        anchors.left: root.left
        anchors.verticalCenter: root.verticalCenter
    }
    Controls.Label {
        id: tokenValueLabel
        horizontalAlignment: Text.AlignRight
        font.weight: Font.Bold
        text: tokenValue && tokenValue.length > 0 ? tokenValue : i18nc("placeholder text if no token is available", "(refresh)")
        anchors.right: root.right
        anchors.verticalCenter: root.verticalCenter
    }
}
