/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import org.kde.kirigami 2.4 as Kirigami

AccountEntryViewBase {
    /*
     * WARNING: AccountEntryViewBase is a derivative of SwipeListItem and SwipeListem instances *must* be
     * called `listItem`. This took *way* too long to figure out. If you change it, things will break for example the
     * flood fill effect when pressing a list entry on Android.
     */
    id: listItem

    property real healthIndicator: 0
    property real interval: listItem.alive ? 1000 * listItem.account.timeStep : 0

    actions: [
        Kirigami.Action {
            iconName: "documentinfo"
            text: i18nc("Button to show details of a single account", "Show details")
            enabled: listItem.alive
            onTriggered: {
                listItem.actionTriggered();
                listItem.details.open();
            }
        },
        Kirigami.Action {
            iconName: "edit-delete"
            text: i18nc("Button for removal of a single account", "Delete account")
            enabled: listItem.alive
            onTriggered: {
                listItem.actionTriggered();
                listItem.sheet.open();
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
        if (listItem.alive && listItem.shouldBeActive) {
            timer.stop()
            timeoutIndicatorAnimation.stop();

            listItem.account.recompute();

            var phase = listItem.account.millisecondsLeftForToken();
            timer.interval = phase;
            listItem.healthIndicator = phase;
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
        labelColor: listItem.labelColor

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
                var phase = listItem.account.millisecondsLeftForToken();

                listItem.healthIndicator = phase;
                timeoutIndicatorAnimation.from = phase;
                timeoutIndicatorAnimation.duration = phase;
                timeoutIndicatorAnimation.restart();
            }
        }

        Rectangle {
            id: health
            // make the indicator sit flush with the bottom edge of the list item
            y: listItem.height - health.height - listItem.bottomPadding
            /*
             * Horizontal positioning of the rectangle relies on clippling of the list item. The idea is to make the health
             * indicator sit flush with the left border while maintaining soft rounded corners on the right (asymmetry).
             *
             * To achieve this simply add a dummy amount of width to the rectangle and compensate for it by offseting it
             * a corresponding amount further to the left; due to clipping the dummy amount will not be shown (but the
             * portion being clipped will contain the rounded corners on the left). The required length for the dummy part
             * depends on the corner radius of the rectangle.
             */
            x: - listItem.leftPadding - health.height
            width: listItem.alive && listItem.interval > 0 ? health.height + listItem.width * listItem.healthIndicator / listItem.interval : 0
            radius: health.height // right edge becomes a semi-circle
            /*
             * Height and opacity are a bit of a balancing act between good looking visuals with few accounts and avoiding
             * an overwhelming UI with many accounts (and therefore many running animations). Opacity is increased when
             * highlighted to get better contrast.
             */
            height: Kirigami.Units.devicePixelRatio * 6
            opacity: timeoutIndicatorAnimation.running ? listItem.highlightActive ? 0.6 : 0.4 : 0
            color: listItem.highlightActive ? listItem.labelColor : Kirigami.Theme.positiveTextColor
            NumberAnimation {
                id: timeoutIndicatorAnimation
                target: listItem
                property: "healthIndicator"
                from: timer.interval
                to: 0
                duration: timer.interval
                running: listItem.alive
            }
            Timer {
                id: timer
                running: listItem.alive
                interval: listItem.alive ? listItem.account.millisecondsLeftForToken() : 0
                onTriggered: {
                    if (listItem.alive) {
                        timer.stop()
                        timeoutIndicatorAnimation.stop();

                        listItem.account.recompute();

                        var phase = listItem.account.millisecondsLeftForToken();
                        timer.interval = phase;
                        listItem.healthIndicator = phase;
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
