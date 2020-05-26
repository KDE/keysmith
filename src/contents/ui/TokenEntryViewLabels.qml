/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

Item {
    property string accountName
    property string tokenValue

    id: root
    /*
     * It appears that Kirigami.SwipeListItem is quite broken (at least on Android): without setting an implicitHeight here
     * the SwipeListItem which will eventually contain this item is not tall enough even for its own actions!
     *
     * The work around is to just sort of guess at what an appropriate implicitHeight might be.
     * Note that on desktop this does approximately nothing, on Android it seems to set the height to
     * Kirigami.Units.iconSizes.small and that appears to be match with the icon size used for SwipeListItem actions there
     * as well.
     */
    implicitHeight: Math.max(accountNameLabel.height, tokenValueLabel.height, Kirigami.Units.iconSizes.small)
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
