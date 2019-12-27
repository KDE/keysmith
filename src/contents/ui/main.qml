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

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models
import Oath.Validators 1.0 as Validators

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    pageStack.initialPage: mainPageComponent

    property bool addActionEnabled: true

    property Models.AccountListModel accounts: Keysmith.accountListModel()

    Kirigami.Action {
        id: addAction
        text: i18n("Add")
        iconName: "list-add"
        visible: addActionEnabled
        onTriggered: {
            pageStack.push(addPageComponent);
            addActionEnabled = false
        }
    }

    Component {
        id: mainListDelegate
        AccountEntryView {
            account: model.account
        }
    }

    Component {
        id: mainPageComponent
        Kirigami.ScrollablePage {
            title: i18nc("@title:window", "Accounts")
            actions.main: addAction
            /*
             * Explicitly opt-out of scroll-to-refresh/drag-to-refresh behaviour
             * Underlying model implementations don't offer the hooks for that.
             */
            supportsRefreshing: false
            ListView {
                id: mainList
                model: accounts
                delegate:  Kirigami.DelegateRecycler {
                    width: parent ? parent.width : implicitWidth
                    sourceComponent: mainListDelegate
                }
            }
        }
    }

    Component {
        id: addPageComponent
        Kirigami.Page {
            title: i18nc("@title:window", "Add new account")
            actions.main: Kirigami.Action {
                text: i18n("Add")
                iconName: "answer-correct"
                onTriggered: {
                    if (tokenDetails.isTotp) {
                        accounts.addTotp(accountName.text, tokenDetails.secret, parseInt(tokenDetails.timeStep), tokenDetails.tokenLength);
                    }
                    if (tokenDetails.isHotp) {
                        accounts.addHotp(accountName.text, tokenDetails.secret, parseInt(tokenDetails.counter), tokenDetails.tokenLength);
                    }

                    pageStack.pop();
                    addActionEnabled = true;
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
                        Kirigami.FormData.label: i18nc("@label:textbox", "Account Name:")
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
