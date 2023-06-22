/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models
import Keysmith.Validators 1.0 as Validators

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

MobileForm.FormCard {
    id: root

    property bool validateAccountAvailability
    property Models.AccountListModel accounts
    property Models.ValidatedAccountInput validatedInput

    onValidatedInputChanged: {
        revalidate();
    }

    onValidateAccountAvailabilityChanged: {
        revalidate();
    }

    property bool acceptable : accountName.acceptableInput && issuerName.acceptableInput

    /*
     * When accounts are added/removed from the model, the account name and issuer should be revalidated as well.
     * It may have become eligible or in-eligible depending on whether or not other accounts with the same name
     * for the same new issuer value now (still) exist.
     *
     * Unfortunately there seems to be nothing to explicitly trigger revalidation on the text field.
     * Work around is to force revalidation to happen by "editing" the value in the text field(s) directly.
     */
    Connections {
        target: accounts
        function onRowsInserted() {
            revalidate();
        }
        function onRowsRemoved() {
            revalidate();
        }
        function onModelReset() {
            revalidate();
        }
    }
    function revalidate() {
        // because of how issuer revalidation works, this also implicitly covers the account name as well
        issuerName.insert(issuerName.text.length, "");
    }
    function forceActiveFocus() {
        accountName.forceActiveFocus()
    }

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormTextFieldDelegate {
            id: accountName
            text: validatedInput.name
            label: i18nc("@label:textbox", "Account name:")
            validator: Validators.AccountNameValidator {
                id: accountNameValidator
                accounts: root.accounts
                issuer: validatedInput.issuer
                validateAvailability: validateAccountAvailability
            }
            onTextChanged: {
                if (acceptableInput) {
                    validatedInput.name = text;
                }
            }
        }

        MobileForm.FormDelegateSeparator {}

        MobileForm.FormTextFieldDelegate {
            id: issuerName
            text: validatedInput.issuer
            label: i18nc("@label:textbox", "Account issuer:")
            validator: Validators.AccountIssuerValidator {}
            /*
             * When the issuer changes, the account name should be revalidated as well.
             * It may have become eligible or in-eligible depending on whether or not other accounts with the same name
             * for the same new issuer value already exist.
             *
             * Unfortunately because the property binding only affects the validator, there seems to be nothing to
             * explicitly trigger revalidation on the text field. Work around is to force revalidation to happen by
             * "editing" the value in the text field directly.
             */
            onTextChanged: {
                /*
                 * This signal handler may run before property bindings have been fully (re-)evaluated.
                 * First update the account name validator to the correct new issuer value before triggering revalidation.
                 */
                accountNameValidator.issuer = issuerName.text;
                accountName.insert(accountName.text.length, "");
                if (acceptableInput) {
                    validatedInput.issuer = text;
                }
            }
        }
    }
}
