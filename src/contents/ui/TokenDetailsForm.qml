/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import Keysmith.Validators 1.0 as Validators
import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.FormLayout {
    id: root
    property bool isTotp: totpRadio.checked && !hotpRadio.checked
    property bool isHotp: hotpRadio.checked && !totpRadio.checked
    property int tokenLength: pinLengthField.value
    property string timeStep: timerField.text
    property string secret: accountSecret.text
    property string counter: counterField.text

    ColumnLayout {
        Layout.rowSpan: 2
        Kirigami.FormData.label: i18nc("@label:chooser", "Account Type:")
        Kirigami.FormData.buddyFor: totpRadio
        Controls.RadioButton {
            id: totpRadio
            checked: true
            text: i18nc("@option:radio", "Time-based OTP")
        }
        Controls.RadioButton {
            id: hotpRadio
            checked: false
            text: i18nc("@option:radio", "Hash-based OTP")
        }
    }
    Controls.TextField {
        id: accountSecret
        text: ""
        Kirigami.FormData.label: i18nc("@label:textbox", "Secret key:")
        validator: Validators.Base32SecretValidator {
            id: secretValidator
        }
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
    }
    Controls.TextField {
        id: timerField
        Kirigami.FormData.label: i18nc("@label:textbox", "Timer:")
        enabled: totpRadio.checked
        text: "30"
        validator: IntValidator {
            bottom: 1
        }
        inputMethodHints: Qt.ImhDigitsOnly
    }
    Controls.TextField {
        id: counterField
        text: "0"
        Kirigami.FormData.label: i18nc("@label:textbox", "Counter:")
        enabled: hotpRadio.checked
        validator: Validators.HOTPCounterValidator {
            id: counterValidator
        }
        inputMethodHints: Qt.ImhDigitsOnly
    }
    /*
     * OATH tokens are derived from a 32bit value, base-10 encoded.
     * Meaning tokens should not be longer than 10 digits max.
     * In addition tokens must be 6 digits long at minimum.
     *
     * Make a virtue of it by offering a spinner for better UX
     */
    Controls.SpinBox {
        id: pinLengthField
        Kirigami.FormData.label: i18nc("@label:spinbox", "Token length:")
        from: 6
        to: 10
        value: 6
    }
}
