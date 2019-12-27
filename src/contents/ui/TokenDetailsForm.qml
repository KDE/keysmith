/*
 * Copyright 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License or any later version accepted by the membership of
 * KDE e.V. (or its successor approved by the membership of KDE
 * e.V.), which shall act as a proxy defined in Section 14 of
 * version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import Oath.Validators 1.0 as Validators
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
        inputMask: "0009"
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
     * The liboath API is documented to support tokens which are
     * 6, 7 or 8 characters long only.
     *
     * Make a virtue of it by offering a spinner for better UX
     */
    Controls.SpinBox {
        id: pinLengthField
        Kirigami.FormData.label: i18nc("@label:spinbox", "Token length:")
        from: 6
        to: 8
        value: 6
    }
}
