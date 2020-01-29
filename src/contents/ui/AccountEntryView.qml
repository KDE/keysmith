/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

import Keysmith.Models 1.0 as Models

Kirigami.SwipeListItem {
    id: root
    property Models.Account account: null
    property int phase : account && account.isTotp ? account.millisecondsLeftForToken() : 0
    property int interval: account && account.isTotp ? 1000 * account.timeStep : 0

    property real healthIndicator: 0

    property Kirigami.Action advanceCounter : Kirigami.Action {
        iconName: "go-next" // "view-refresh"
        text: "Next token"
        onTriggered: {
            // TODO convert to C++ helper, have proper logging?
            if (account && account.isHotp) {
                account.advanceCounter(1);
            }
            // TODO warn if not
        }
    }

    property Kirigami.Action deleteAccount : Kirigami.Action {
        iconName: "edit-delete"
        text: "Delete account"
        onTriggered: {
            // TODO convert to C++ helper, have proper logging?
            if (account) {
                account.remove();
            }
            // TODO warn if not
        }
    }

    actions: account && account.isHotp ? [deleteAccount, advanceCounter] : [deleteAccount]

    contentItem: ColumnLayout {
        id: mainLayout
        RowLayout {
            Controls.Label {
                id: accountNameLabel
                horizontalAlignment: Text.AlignLeft
                font.weight: Font.Light
                elide: Text.ElideRight
                text: account ? account.name : i18nc("placeholder text if no account name is available", "(untitled)")
            }
            Controls.Label {
                id: tokenValueLabel
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true
                font.weight: Font.Bold
                text: account && account.token && account.token.length > 0 ? account.token : i18nc("placeholder text if no token is available", "(refresh)")
            }
        }
        Timer {
            id: timer
            running: account && account.isTotp
            interval: phase
            onTriggered: {
                // TODO convert to C++ helper, have proper logging?
                if (account) {
                    if (account.isTotp) {
                        timer.stop()
                        timeoutIndicatorAnimation.stop();

                        account.recompute();
                        timer.interval = account.millisecondsLeftForToken(); // root.interval;
                        timer.restart();
                        timeoutIndicatorAnimation.restart();
                    }
                }
                // TODO warn if not
            }
        }
        Rectangle {
            id: health
            Layout.fillWidth: true
            color: Kirigami.Theme.positiveTextColor
            height: Kirigami.Units.smallSpacing
            opacity: timer.running ? 0.6 : 0
            radius: health.height
            /*
             * Don't use mainLayout.width because that doesn't seem to be affected by hovering which uncovers 'hidden'
             * action buttons. Compute the correct width manually, to avoid 'flashes' where the health indicator may
             * appear to be 'reset' to (near) full width.
             */
            width: (accountNameLabel.width + tokenValueLabel.width)  * healthIndicator
            NumberAnimation {
                id: timeoutIndicatorAnimation
                /*
                 * Don't animate the rectangle directly: instead animate a proxy property to track the desired ratio.
                 * This way the property binding for the width of the rectangle is fully re-evaluated whenever the
                 * app window size changes. In turn, that then ensures the health indicator rectangle is also resized
                 * accordingly to the correct proportion of the new width of the layout.
                 */
                target: root
                property: "healthIndicator"
                from: timer.interval / root.interval
                to: 0
                duration: timer.interval
                running: model.account && model.account.isTotp && Kirigami.Units.longDuration > 1
            }
        }
    }
}
