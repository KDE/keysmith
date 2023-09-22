/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1 as FormCard

import Keysmith.Models as Models
import Keysmith.Validators as Validators

FormCard.FormCard {
    id: root

    property Models.ValidatedAccountInput validatedInput

    property bool counterAcceptable: counterField.acceptableInput
    property bool acceptable: counterAcceptable

    FormCard.FormTextFieldDelegate {
        id: counterField

        text: validatedInput ? validatedInput.counter : ""
        label: i18nc("@label:textbox", "Counter:")
        validator: Validators.HOTPCounterValidator {}
        inputMethodHints: Qt.ImhDigitsOnly
        onTextChanged: {
            if (acceptableInput) {
                validatedInput.setCounter(text, validator);
            }
        }
    }

    FormCard.FormDelegateSeparator {}

    /*
     * OATH tokens are derived from a 32bit value, base-10 encoded.
     * Meaning tokens should not be longer than 10 digits max.
     * In addition tokens must be 6 digits long at minimum.
     *
     * Make a virtue of it by offering a spinner for better UX
     */
    FormCard.FormSpinBoxDelegate {
        id: tokenLengthField
        label: i18nc("@label:spinbox", "Token length:")
        from: 6
        to: 10
        value: validatedInput ? validatedInput.tokenLength : 6
        onValueChanged: {
            validatedInput.tokenLength = value;
        }
    }

    FormCard.FormDelegateSeparator { above: checksumField }

    FormCard.FormCheckDelegate {
        id: checksumField

        checked: validatedInput.checksum
        text: i18nc("@option:check", "Add checksum digit")
        onCheckedChanged: {
            validatedInput.checksum = checked;
        }
    }

    FormCard.FormDelegateSeparator { below: checksumField }

    FormCard.FormCheckDelegate {
        id: truncationTypeField

        checked: validatedInput && validatedInput.fixedTruncation
        text: i18nc("@option:check", "Use custom truncation offset")
        onCheckedChanged: {
            if (!checked) {
                validatedInput.setDynamicTruncation();
            }
        }
    }

    FormCard.FormDelegateSeparator { below: truncationTypeField }

    FormCard.FormSpinBoxDelegate {
        id: truncationValueField

        enabled: truncationTypeField.checked
        label: i18nc("@label:spinbox", "Truncation offset:")
        from: 0
        /*
         * HOTP tokens are based on a HMAC-SHA1 construction yielding 20 bytes of output of which 4 are taken to
         * compute the token value.
         */
        to: 16
        value: validatedInput ? validatedInput.truncationOffset : 0
    }
}
