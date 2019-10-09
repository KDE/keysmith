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

import Oath 1.0
import Oath.Validators 1.0 as Validators
import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: "OTP Client"

    pageStack.initialPage: accounts.rowCount() > 0 ? mainPageComponent : addPageComponent

    property bool addActionEnabled: true

    AccountModel {
        id: accounts
    }

    Kirigami.Action {
        id: addAction
        text: "Add"
        iconName: "list-add"
        visible: addActionEnabled
        onTriggered: {
            pageStack.push(addPageComponent);
            addActionEnabled = false
        }
    }

    Component {
        id: mainPageComponent
        Kirigami.ScrollablePage {
            title: "OTP"
            actions.main: addAction
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
                    newAccount.type = tokenDetails.type
                    newAccount.secret = tokenDetails.secret
                    newAccount.counter = parseInt(tokenDetails.counter)
                    newAccount.timeStep = parseInt(tokenDetails.timeStep)
                    newAccount.pinLength = parseInt(tokenDetails.tokenLength)

                    pageStack.pop();
                    addActionEnabled = true;
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

            ColumnLayout {
                id: addFormLayout
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                Kirigami.FormLayout {

                    Controls.TextField {
                        id: accountName
                        Kirigami.FormData.label: "Account Name:"
                        validator: Validators.AccountNameValidator {
                            id: nameValidator
                        }
                    }
                }
                TokenDetailsForm {
                    id: tokenDetails
                }
            }
        }
    }
}
