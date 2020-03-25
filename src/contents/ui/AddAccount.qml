/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models
import Keysmith.Validators 1.0 as Validators

Kirigami.Page {
    id: root
    title: i18nc("@title:window", "Add new account")
    signal dismissed
    property Models.AccountListModel accounts: Keysmith.accountListModel()
    property bool acceptable: accountName.acceptableInput && tokenDetails.acceptable

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        Kirigami.FormLayout {
            Controls.TextField {
                id: accountName
                Kirigami.FormData.label: i18nc("@label:textbox", "Account Name:")
                validator: Validators.AccountNameValidator {
                    accounts: root.accounts
                }
            }
        }
        TokenDetailsForm {
            id: tokenDetails
        }
    }

    actions.main: Kirigami.Action {
        text: i18n("Add")
        iconName: "answer-correct"
        enabled: acceptable
        onTriggered: {
            if (tokenDetails.isTotp) {
                accounts.addTotp(accountName.text, tokenDetails.secret, parseInt(tokenDetails.timeStep), tokenDetails.tokenLength);
            }
            if (tokenDetails.isHotp) {
                accounts.addHotp(accountName.text, tokenDetails.secret, parseInt(tokenDetails.counter), tokenDetails.tokenLength);
            }
            root.dismissed();
        }
    }
}
