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
    property bool acceptable: accountName.acceptableInput && issuerName.acceptableInput && tokenDetails.acceptable

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        Kirigami.FormLayout {
            Controls.TextField {
                id: accountName
                Kirigami.FormData.label: i18nc("@label:textbox", "Account Name:")
                validator: Validators.AccountNameValidator {
                    id: accountNameValidator
                    accounts: root.accounts
                    issuer: issuerName.text
                }
            }
            Controls.TextField {
                id: issuerName
                Kirigami.FormData.label: i18nc("@label:textbox", "Account Issuer:")
                validator: Validators.AccountIssuerValidator {}
                /*
                 * When the issuer changes, the account name should be revalidated as well. It may have become eligible or in-eligible depending on
                 * whether or not other accounts with the same name for the same new issuer value already exist.
                 *
                 * Unfortunately because the property binding only affects the validator, there seems to be nothing to explicitly trigger revalidation
                 * on the text field. Work around is to force revalidation to happen by "editing" the value in the text field directly.
                 */
                onTextChanged: {
                    /*
                     * This signal handler may run before property bindings have been fully (re-)evaluated.
                     * First update the account name validator to the correct new issuer value before triggering revalidation.
                     */
                    accountNameValidator.issuer = issuerName.text;
                    accountName.insert(accountName.text.length, "");
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
                console.log("WTF: ", Models.AccountListModel.Sha1);
                accounts.addTotp(accountName.text, issuerName.text, tokenDetails.secret, tokenDetails.tokenLength, parseInt(tokenDetails.timeStep), new Date(0), Models.AccountListModel.Sha1);
            }
            if (tokenDetails.isHotp) {
                accounts.addHotp(accountName.text, issuerName.text, tokenDetails.secret, tokenDetails.tokenLength, parseInt(tokenDetails.counter), false, 0, false);
            }
            root.dismissed();
        }
    }
}
