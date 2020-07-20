/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
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

    property bool secretAcceptable: accountSecret.acceptableInput
    property bool timeStepAcceptable: timeStepField.acceptableInput || hotpRadio.checked
    property bool counterAcceptable: counterField.acceptableInput || totpRadio.checked
    property bool tokenTypeAcceptable: hotpRadio.checked || totpRadio.checked
    property bool acceptable: counterAcceptable && timeStepAcceptable && secretAcceptable && tokenTypeAcceptable

    ColumnLayout {
        Layout.rowSpan: 2
        Kirigami.FormData.label: i18nc("@label:chooser", "Account Type:")
        Kirigami.FormData.buddyFor: totpRadio
        Controls.RadioButton {
            id: totpRadio
            checked: validatedInput && validatedInput.type === Models.ValidatedAccountInput.Totp
            text: i18nc("@option:radio", "Time-based OTP")
            onCheckedChanged: {
                if (checked) {
                    validatedInput.type = Models.ValidatedAccountInput.Totp;
                }
            }
        }
        Controls.RadioButton {
            id: hotpRadio
            checked: validatedInput && validatedInput.type === Models.ValidatedAccountInput.Hotp
            text: i18nc("@option:radio", "Hash-based OTP")
            onCheckedChanged: {
                if (checked) {
                    validatedInput.type = Models.ValidatedAccountInput.Hotp;
                }
            }
        }
    }
    Kirigami.PasswordField {
        id: accountSecret
        placeholderText: i18n("Token secret")
        text: validatedInput ? validatedInput.secret : ""
        Kirigami.FormData.label: i18nc("@label:textbox", "Secret key:")
        validator: Validators.Base32SecretValidator {
            id: secretValidator
        }
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
        onTextChanged: {
            if (acceptableInput) {
                validatedInput.secret = text;
            }
        }
    }
    Controls.TextField {
        id: timeStepField
        Kirigami.FormData.label: i18nc("@label:textbox", "Timer:")
        enabled: validatedInput.type === Models.ValidatedAccountInput.Totp
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
        id: counterField
        text: validatedInput ? validatedInput.counter : ""
        Kirigami.FormData.label: i18nc("@label:textbox", "Counter:")
        enabled: hotpRadio.checked
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
