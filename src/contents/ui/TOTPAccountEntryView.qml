/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import org.kde.kirigami 2.4 as Kirigami

AccountEntryViewBase {
    id: root

    property real healthIndicator: 0
    property real interval: root.alive ? 1000 * root.account.timeStep : 0

    actions: [
        Kirigami.Action {
            iconName: "edit-delete"
            text: i18nc("Button for removal of a single account", "Delete account")
            enabled: root.alive
            onTriggered: {
                root.actionTriggered();
                root.sheet.open();
            }
        }
    ]

    /*
     * If the application is suspended the displayed state may be out-of-date by the time the application is woken from
     * suspend again. Use a property to monitor for this condition and recover when the application wakes: reset timers,
     * animations and recompute token in case it has lapsed.
     */
    property bool shouldBeActive: Qt.application.state === Qt.ApplicationActive
    onShouldBeActiveChanged: {
        if (root.alive && root.shouldBeActive) {
            timer.stop()
            timeoutIndicatorAnimation.stop();

            root.account.recompute();

            var phase = root.account.millisecondsLeftForToken();
            timer.interval = phase;
            root.healthIndicator = phase;
            timeoutIndicatorAnimation.duration = phase;
            timeoutIndicatorAnimation.from = phase;

            timer.restart();
            timeoutIndicatorAnimation.restart();
        }
    }

    TokenEntryViewLabels {
        id: mainLayout
        accountName: account.name
        tokenValue: account.token
        labelColor: root.labelColor

        /*
         * For some reason the running NumberAnimation seems to trigger very sluggish QML UI when the window is resized.
         * This behaviour persists until the animation is 'reset', so work around by resetting the animation whenever this
         * could have occurred. The easiest proxy for detecting this is whenever the width of the content item changes.
         *
         * Note that this work-around triggers a lot of false positive 'hits' as well: hovering the cursor over the UI
         * also triggers a change on the width property.
         *
         * The particular sluggish behaviour of QML can be reproduced using the following steps:
         *
         *  - commenting out the signal handler
         *  - rebuilding the app and starting it
         *  - with some (multiple) accounts pre-defined, with at least one TOTP account
         *  - resize the app while a health indicator animation is running
         *  - hovering over account entries: observe how QML takes a while to 'catch' up with the cursor, to display the
         *    hover effect in the accounts list view.
         */
        onWidthChanged: {
            if (timeoutIndicatorAnimation.running) {
                timeoutIndicatorAnimation.stop();
                var phase = root.account.millisecondsLeftForToken();

                root.healthIndicator = phase;
                timeoutIndicatorAnimation.from = phase;
                timeoutIndicatorAnimation.duration = phase;
                timeoutIndicatorAnimation.restart();
            }
        }

        Rectangle {
            id: health
            x: - root.leftPadding
            y: root.height - health.height - root.bottomPadding
            radius: health.height
            height: Kirigami.Units.smallSpacing
            opacity: timeoutIndicatorAnimation.running ? 0.6 : 0
            width: root.alive && root.interval > 0 ? root.width * root.healthIndicator / root.interval : 0
            color: Kirigami.Theme.positiveTextColor
            NumberAnimation {
                id: timeoutIndicatorAnimation
                target: root
                property: "healthIndicator"
                from: timer.interval
                to: 0
                duration: timer.interval
                running: root.alive
            }
            Timer {
                id: timer
                running: root.alive
                interval: root.alive ? root.account.millisecondsLeftForToken() : 0
                onTriggered: {
                    if (root.alive) {
                        timer.stop()
                        timeoutIndicatorAnimation.stop();

                        root.account.recompute();

                        var phase = root.account.millisecondsLeftForToken();
                        timer.interval = phase;
                        root.healthIndicator = phase;
                        timeoutIndicatorAnimation.duration = phase;
                        timeoutIndicatorAnimation.from = phase;

                        timer.restart();
                        timeoutIndicatorAnimation.restart();
                    }
                }
            }
        }
    }
}
