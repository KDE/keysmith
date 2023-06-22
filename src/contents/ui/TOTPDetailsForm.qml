/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import Keysmith.Models 1.0 as Models
import Keysmith.Validators 1.0 as Validators

MobileForm.FormCard {
    id: root
    property Models.ValidatedAccountInput validatedInput

    property bool timeStepAcceptable: timeStepField.acceptableInput
    property bool algorithmAcceptable: sha1Radio.checked || sha256Radio.checked || sha512Radio.checked
    property bool epochAcceptable: epochField.acceptableInput
    property bool acceptable: timeStepAcceptable && algorithmAcceptable && epochAcceptable

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18nc("@label", "TOTP Details:")
        }

        MobileForm.FormTextFieldDelegate {
            id: timeStepField

            label: i18nc("@label:textbox", "Timer:")
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

        MobileForm.FormDelegateSeparator {}

        MobileForm.FormTextFieldDelegate {
            id: epochField

            label: i18nc("@label:textbox", "Starting at:")
            text: validatedInput ? validatedInput.epoch : ""
            validator: Validators.TOTPEpochValidator {}
            onTextChanged: {
                if (acceptableInput) {
                    validatedInput.epoch = text;
                }
            }
        }

        MobileForm.FormDelegateSeparator {}

        /*
         * OATH tokens are derived from a 32bit value, base-10 encoded.
         * Meaning tokens should not be longer than 10 digits max.
         * In addition tokens must be 6 digits long at minimum.
         *
         * Make a virtue of it by offering a spinner for better UX
         */
        MobileForm.FormSpinBoxDelegate {
            id: tokenLengthField

            label: i18nc("@label:spinbox", "Token length:")
            from: 6
            to: 10
            value: validatedInput ? validatedInput.tokenLength : 6
            onValueChanged: {
                validatedInput.tokenLength = value;
            }
        }

        MobileForm.FormDelegateSeparator {}

        MobileForm.FormTextDelegate {
            text: i18nc("@label:chooser", "Hash algorithm:")
        }

        MobileForm.FormRadioDelegate {
            id: sha1Radio

            checked: validatedInput.algorithm === Models.ValidatedAccountInput.Sha1
            text: i18nc("@option:radio", "SHA-1")
            onCheckedChanged: {
                if (checked) {
                    validatedInput.algorithm = Models.ValidatedAccountInput.Sha1;
                }
            }
        }

        MobileForm.FormRadioDelegate {
            id: sha256Radio

            checked: validatedInput.algorithm === Models.ValidatedAccountInput.Sha256
            text: i18nc("@option:radio", "SHA-256")
            onCheckedChanged: {
                if (checked) {
                    validatedInput.algorithm = Models.ValidatedAccountInput.Sha256;
                }
            }
        }

        MobileForm.FormRadioDelegate {
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
