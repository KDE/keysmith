/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import Keysmith.Models 1.0 as Models
import Keysmith.Validators 1.0 as Validators
import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

Kirigami.FormLayout {
    id: root
    property Models.ValidatedAccountInput validatedInput

    property bool counterAcceptable: counterField.acceptableInput
    property bool acceptable: counterAcceptable

    Controls.TextField {
        id: counterField
        text: validatedInput ? validatedInput.counter : ""
        Kirigami.FormData.label: i18nc("@label:textbox", "Counter:")
        validator: Validators.HOTPCounterValidator {
            id: counterValidator
        }
        inputMethodHints: Qt.ImhDigitsOnly
        onTextChanged: {
            if (acceptableInput) {
                validatedInput.setCounter(text, validator);
            }
        }
    }
    /*
     * OATH tokens are derived from a 32bit value, base-10 encoded.
     * Meaning tokens should not be longer than 10 digits max.
     * In addition tokens must be 6 digits long at minimum.
     *
     * Make a virtue of it by offering a spinner for better UX
     */
    Controls.SpinBox {
        id: tokenLengthField
        Kirigami.FormData.label: i18nc("@label:spinbox", "Token length:")
        from: 6
        to: 10
        value: validatedInput ? validatedInput.tokenLength : 6
        onValueChanged: {
            validatedInput.tokenLength = value;
        }
    }
}
