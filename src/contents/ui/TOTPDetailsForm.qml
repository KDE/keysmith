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

    property bool timeStepAcceptable: timeStepField.acceptableInput
    property bool algorithmAcceptable: sha1Radio.checked || sha256Radio.checked || sha512Radio.checked
    property bool epochAcceptable: epochField.acceptableInput
    property bool acceptable: timeStepAcceptable && algorithmAcceptable && epochAcceptable

    Controls.TextField {
        id: timeStepField
        Kirigami.FormData.label: i18nc("@label:textbox", "Timer:")
        text: validatedInput ? "" + validatedInput.timeStep : ""
        validator: IntValidator {
            bottom: 1
        }
        inputMethodHints: Qt.ImhDigitsOnly
        onTextChanged: {
            if (acceptableInput) {
                validatedInput.timeStep = parseInt(text);
            }
        }
    }

    Controls.TextField {
        id: epochField
        Kirigami.FormData.label: i18nc("@label:textbox", "Starting at:")
        text: validatedInput ? validatedInput.epoch : ""
        validator: Validators.TOTPEpochValidator {
        }
        onTextChanged: {
            if (acceptableInput) {
                validatedInput.epoch = text;
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

    ColumnLayout {
        Layout.rowSpan: 3
        Kirigami.FormData.label: i18nc("@label:chooser", "Hash algorithm:")
        Kirigami.FormData.buddyFor: sha1Radio
        Controls.RadioButton {
            id: sha1Radio
            checked: validatedInput.algorithm === Models.ValidatedAccountInput.Sha1
            text: i18nc("@option:radio", "SHA-1")
            onCheckedChanged: {
                if (checked) {
                    validatedInput.algorithm = Models.ValidatedAccountInput.Sha1;
                }
            }
        }
        Controls.RadioButton {
            id: sha256Radio
            checked: validatedInput.algorithm === Models.ValidatedAccountInput.Sha256
            text: i18nc("@option:radio", "SHA-256")
            onCheckedChanged: {
                if (checked) {
                    validatedInput.algorithm = Models.ValidatedAccountInput.Sha256;
                }
            }
        }
        Controls.RadioButton {
            id: sha512Radio
            checked: validatedInput.algorithm === Models.ValidatedAccountInput.Sha512
            text: i18nc("@option:radio", "SHA-512")
            onCheckedChanged: {
                if (checked) {
                    validatedInput.algorithm = Models.ValidatedAccountInput.Sha512;
                }
            }
        }
    }
}
