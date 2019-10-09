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

import Oath 1.0
import Oath.Validators 1.0 as Validators
import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.FormLayout {
    id: root
    property int type: totpRadio.checked ? Account.TypeTOTP : Account.TypeHOTP
    property int tokenLength: pinLengthField.value
    property string timeStep: timerField.text
    property string secret: accountSecret.text
    property string counter: counterField.text

    property Account account: null
    property bool editable: false

    ColumnLayout {
        Layout.rowSpan: 2
        Kirigami.FormData.label: "Account Type:"
        Kirigami.FormData.buddyFor: totpRadio
        Controls.RadioButton {
            id: totpRadio
            checked: !account || account.type == Account.TypeTOTP
            text: "Time-based OTP"
        }
        Controls.RadioButton {
            id: hotpRadio
            checked: account && account.type == Account.TypeHOTP
            text: "Hash-based OTP"
        }
    }
    Controls.TextField {
        id: accountSecret
        text: account ? account.secret : ""
        Kirigami.FormData.label: "Secret key:"
        validator: Validators.Base32SecretValidator {
            id: secretValidator
        }
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
    }
    Controls.TextField {
        id: timerField
        Kirigami.FormData.label: "Timer:"
        enabled: totpRadio.checked
        text: account ? "" + account.timeStep : "30"
        inputMask: "0009"
        inputMethodHints: Qt.ImhDigitsOnly
    }
    Controls.TextField {
        id: counterField
        text: account ? "" + account.counter : ""
        Kirigami.FormData.label: "Counter:"
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
        Kirigami.FormData.label: "Token length:"
        from: 6
        to: 8
        value: account ? account.pinLength : 6
    }
}
