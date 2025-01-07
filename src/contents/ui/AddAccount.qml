/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1 as FormCard

import Keysmith.Application as Application
import Keysmith.Models as Models
import Keysmith.Validators as Validators


FormCard.FormCardPage {
    id: root

    required property Application.AddAccountViewModel vm

    title: i18nc("@title:window", "Add new account")

    property Models.ValidatedAccountInput validatedInput

    property bool detailsEnabled: false

    property bool secretAcceptable: accountSecret.acceptableInput
    property bool tokenTypeAcceptable: hotpRadio.checked || totpRadio.checked
    property bool hotpDetailsAcceptable: hotpDetails.acceptable || vm.input.type === Models.ValidatedAccountInput.Totp
    property bool totpDetailsAcceptable: totpDetails.acceptable || vm.input.type === Models.ValidatedAccountInput.Hotp
    property bool tokenDetailsAcceptable: hotpDetailsAcceptable && totpDetailsAcceptable
    property bool acceptable: accountName.acceptable && secretAcceptable && tokenTypeAcceptable && tokenDetailsAcceptable

    topPadding: Kirigami.Units.gridUnit
    bottomPadding: Kirigami.Units.gridUnit

    data: Connections {
        target: vm.input
        function onTypeChanged() {
            root.detailsEnabled = false;
        }
    }

    QrCodeScanner {
        id: scanner

        onAccepted: {
            // Then scanner.ou (OptUri) is valid and we can use it to fill in the data.
            if (!scanner.value.valid) {
                return;
            }

            vm.input.type = scanner.value.isHOtp ? Models.ValidatedAccountInput.Hotp
                                                 : Models.ValidatedAccountInput.Totp;

            // Otherwise it will not work, due to validation stuff.
            accountSecret.clear();
            accountSecret.insert(0, scanner.value.secret);

            accountName.validatedInput.name = scanner.value.account;
            accountName.issuer = scanner.value.issuer;

            validatedInput.timer = scanner.value.period;
            validatedInput.counter = scanner.value.counter;
            validatedInput.epoch = "1970-01-01T00:00:00Z"; // Because is standard.
            validatedInput.tokenLength = scanner.value.digits;

            if (scanner.value.algorithm == "SHA1") {
                validatedInput.algorithm = Models.ValidatedAccountInput.Sha1;
            } else if (scanner.value.algorithm == "SHA256") {
                validatedInput.algorithm = Models.ValidatedAccountInput.Sha256;
            } else if (scanner.value.algorithm == "SHA512") {
                validatedInput.algorithm = Models.ValidatedAccountInput.Sha512;
            }
        }
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button cancel and dismiss the add account form", "Cancel")
            icon.name: "edit-undo"
            onTriggered: {
                vm.cancelled();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button Dismiss the error page and quit Keysmith", "Quit")
            icon.name: "application-exit"
            enabled: vm.quitEnabled
            visible: vm.quitEnabled
            onTriggered: {
                Qt.quit();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Scan from camera")
            icon.name: "view-barcode-qr"
            onTriggered: {
                scanner.open();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Add")
            icon.name: "answer-correct"
            enabled: acceptable
            onTriggered: {
                vm.accepted();
            }
        }
    ]

    Component.onCompleted: accountName.forceActiveFocus()

    AccountNameForm {
        id: accountName
        Layout.fillWidth: true
        accounts: vm.accounts
        validateAccountAvailability: vm.validateAvailability
        validatedInput: vm.input
    }

    FormCard.FormCard {
        id: qrSelector
    }

    FormCard.FormHeader {
        title: i18nc("@label:chooser", "Account type:")
    }

    FormCard.FormCard {
        id: requiredDetails

        FormCard.FormRadioDelegate {
            id: totpRadio
            checked: vm.input.type === Models.ValidatedAccountInput.Totp
            text: i18nc("@option:radio", "Time-based OTP")
            onCheckedChanged: {
                if (checked) {
                    vm.input.type = Models.ValidatedAccountInput.Totp;
                }
            }
        }

        FormCard.FormRadioDelegate {
            id: hotpRadio
            checked: vm.input.type === Models.ValidatedAccountInput.Hotp
            text: i18nc("@option:radio", "Hash-based OTP")
            onCheckedChanged: {
                if (checked) {
                    vm.input.type = Models.ValidatedAccountInput.Hotp;
                }
            }
        }

        FormCard.FormDelegateSeparator { below: hotpRadio }

        FormCard.FormTextFieldDelegate {
            id: accountSecret
            placeholderText: i18n("Token secret")
            text: vm.input.secret
            echoMode: TextInput.Password
            label: i18nc("@label:textbox", "Secret key:")
            validator: Validators.Base32SecretValidator {
                id: secretValidator
            }
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
            onTextChanged: {
                if (acceptableInput) {
                    vm.input.secret = text;
                }
            }
        }

        FormCard.FormDelegateSeparator { above: toggleDetails }

        FormCard.FormButtonDelegate {
            id: toggleDetails
            text: i18nc("Button to reveal form for configuring additional token details", "Details")
            enabled: !root.detailsEnabled && (hotpRadio.checked || totpRadio.checked)
            visible: enabled
            onClicked: root.detailsEnabled = true;
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "HOTP Details:")
        visible: hotpDetails.visible
    }

    HOTPDetailsForm {
        id: hotpDetails

        visible: enabled
        validatedInput: root.vm.input
        enabled: root.detailsEnabled && vm.input.type === Models.ValidatedAccountInput.Hotp
    }

    FormCard.FormHeader {
        title: i18nc("@label", "TOTP Details:")
        visible: totpDetails.visible
    }

    TOTPDetailsForm {
        id: totpDetails

        visible: enabled
        validatedInput: root.vm.input
        enabled: root.detailsEnabled && vm.input.type === Models.ValidatedAccountInput.Totp
    }
}
