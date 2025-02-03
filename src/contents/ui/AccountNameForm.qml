/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import Keysmith.Application
import Keysmith.Models as Models
import Keysmith.Validators as Validators

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1 as FormCard

FormCard.FormCard {
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

    Component.onCompleted: {
        doValidation = true;
    }

    property bool acceptable : accountName.acceptableInput && issuerName.acceptableInput

    /**
     * Property used to delay validation until the fields have been set to their initial values.
     * Otherwise the validator sets the issuer to an empty string, which overwrites the initial isser.
     */
    property bool doValidation: false

    /*
     * When accounts are added/removed from the model, the account name and issuer should be revalidated as well.
     * It may have become eligible or in-eligible depending on whether or not other accounts with the same name
     * for the same new issuer value now (still) exist.
     *
     * Unfortunately there seems to be nothing to explicitly trigger revalidation on the text field.
     * Work around is to force revalidation to happen by "editing" the value in the text field(s) directly.
     */
    data: [
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
        },
        Binding {
            root.validatedInput.issuer: issuerName.text
        }
    ]
    function revalidate() {
        // because of how issuer revalidation works, this also implicitly covers the account name as well
        if (doValidation) {
            issuerName.insert(issuerName.text.length, "");
        }
    }
    function forceActiveFocus() {
        accountName.forceActiveFocus()
    }

    FormCard.FormTextFieldDelegate {
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

    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
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
            accountNameValidator.issuer = text;
            accountName.insert(accountName.text.length, "");
            if (acceptableInput) {
                validatedInput.issuer = text;
            }
        }
    }
}
