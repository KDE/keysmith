/*
 * Copyright 2019 Bhushan Shah <bshah@kde.org>
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

import OAth 1.0
import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: "OTP Client"

    pageStack.initialPage: accounts.rowCount() > 0 ? mainPageComponent : addPageComponent

    AccountModel {
        id: accounts
    }

    Component {
        id: mainPageComponent
        Kirigami.ScrollablePage {
            title: "OTP"
            actions.main: Kirigami.Action {
                text: "Add"
                iconName: "list-add"
                onTriggered: {
                    pageStack.push(addPageComponent);
                }
            }
            Controls.Label {
                text: "No account set up. Use the add button to add accounts."
                visible: view.count == 0
            }
            Kirigami.CardsListView {
                id: view
                model: accounts
                delegate: Kirigami.AbstractCard {
                    contentItem: Item {
                        implicitWidth: delegateLayout.implicitWidth
                        implicitHeight: delegateLayout.implicitHeight
                        GridLayout {
                            id: delegateLayout
                            anchors {
                                left: parent.left
                                top: parent.top
                                right: parent.right
                                //IMPORTANT: never put the bottom margin
                            }
                            rowSpacing: Kirigami.Units.largeSpacing
                            columnSpacing: Kirigami.Units.largeSpacing
                            columns: width > Kirigami.Units.gridUnit * 20 ? 4 : 2
                            ColumnLayout {
                                Controls.Label {
                                    Layout.fillWidth: true
                                    text: model.name
                                }
                                Kirigami.Heading {
                                    level: 2
                                    text: model.otp
                                    onTextChanged: {
                                        if(model.type === Account.TypeTOTP) {
                                            timeoutTimer.restart();
                                        }
                                    }
                                }
                            }
                            Controls.Button {
                                Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                                Layout.columnSpan: 2
                                text: "Refresh (" + model.counter + ")"
                                visible: model.type === Account.TypeHOTP
                                onClicked: {
                                    accounts.generateNext(index);
                                }
                            }
                            Timer {
                                id: timeoutTimer
                                repeat: true
                                interval: model.timeStep * 1000
                                running: model.type === Account.TypeTOTP
                            }
                            Rectangle {
                                id: timeoutIndicatorRect
                                Layout.fillHeight: true
                                width: 5
                                radius: width
                                color: "green"
                                visible: timeoutTimer.running && units.longDuration > 1
                                opacity: timeoutIndicatorAnimation.running ? 0.6 : 0
                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: units.longDuration
                                    }
                                }
                            }
                            NumberAnimation {
                                id: timeoutIndicatorAnimation
                                target: timeoutIndicatorRect
                                property: "height"
                                from: delegateLayout.height
                                to: 0
                                duration: timeoutTimer.interval
                                running: model.type === Account.TypeTOTP && units.longDuration > 1
                            }
                        }
                    }
                }
            }
        }
    }
    Component {
        id: addPageComponent
        Kirigami.Page {
            title: "Add new OTP"
            actions.main: Kirigami.Action {
                text: "Add"
                iconName: "answer-correct"
                onTriggered: {
                    var newAccount = accounts.createAccount();
                    newAccount.name = accountName.text;
                    newAccount.type = totpRadio.checked ? Account.TypeTOTP : Account.TypeHOTP
                    newAccount.secret = accountSecret.text
                    newAccount.counter = parseInt(counterField.text)
                    newAccount.timeStep = parseInt(timerField.text)
                    newAccount.pinLength = parseInt(pinLengthField.text)

                    pageStack.pop();
                    /*
                     * Check if the pageStack is now 'empty', which will be the case if
                     * the starting page was this addPageComponent.
                     *
                     * According to Qt docs the StackView 'empty' property is supposed to exist
                     * and be a bool but in practice it does not appear to work (it is undefined).
                     * Therefore check the StackView.depth instead.
                     */
                    if (pageStack.depth < 1) {
                        pageStack.push(mainPageComponent);
                    }
                }
            }
            Kirigami.FormLayout {
                id: layout
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                Controls.TextField {
                    id: accountName
                    Kirigami.FormData.label: "Account Name:"
                }
                ColumnLayout {
                    Layout.rowSpan: 2
                    Kirigami.FormData.label: "Account Type:"
                    Kirigami.FormData.buddyFor: totpRadio
                    Controls.RadioButton {
                        id: totpRadio
                        checked: true
                        text: "Time-based OTP"
                    }
                    Controls.RadioButton {
                        id: hotpRadio
                        text: "Hash-based OTP"
                    }
                }
                Controls.TextField {
                    id: accountSecret
                    Kirigami.FormData.label: "Secret key:"
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                }
                Controls.TextField {
                    id: timerField
                    Kirigami.FormData.label: "Timer:"
                    enabled: totpRadio.checked
                    text: "30"
                    inputMask: "0009"
                    inputMethodHints: Qt.ImhDigitsOnly
                }
                Controls.TextField {
                    id: counterField
                    Kirigami.FormData.label: "Counter:"
                    enabled: hotpRadio.checked
                    inputMask: "0009"
                    inputMethodHints: Qt.ImhDigitsOnly
                }
                /*
                 * The liboath API is documented to support tokens which are
                 * 6, 7 or 8 characters long only.
                 *
                 * Make a virtue of it by offering a spinner for better UX
                 */
                Controls.SpinBox {
                    id: pinLengthField
                    Kirigami.FormData.label: "Token length:"
                    from: 6
                    to: 8
                    value: 6
                }
            }
        }
    }
}
