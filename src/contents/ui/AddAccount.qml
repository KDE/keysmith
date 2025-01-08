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
        target: root.vm.input
        function onTypeChanged() {
            root.detailsEnabled = false;
        }
    }

    function fillInput(value) {
        root.vm.input.type = value.isHOtp ? Models.ValidatedAccountInput.Hotp
                                          : Models.ValidatedAccountInput.Totp;

        // Otherwise it will not work, due to validation stuff.
        accountSecret.clear();
        accountSecret.insert(0, value.secret);

        accountName.issuer.clear();
        accountName.issuer.insert(0, value.issuer);
        accountName.account.clear();
        accountName.account.insert(0, value.account);

        validatedInput.timer = value.period;
        validatedInput.counter = value.counter;
        validatedInput.epoch = "1970-01-01T00:00:00Z"; // Because is standard.
        validatedInput.tokenLength = value.digits;

        const algo = value.algorithm;
        if (algo == "SHA1") {
            validatedInput.algorithm = Models.ValidatedAccountInput.Sha1;
        } else if (algo == "SHA256") {
            validatedInput.algorithm = Models.ValidatedAccountInput.Sha256;
        } else if (algo == "SHA512") {
            validatedInput.algorithm = Models.ValidatedAccountInput.Sha512;
        }
    }

    QrCodeImageScanner {
        id: imageScanner

        onAccepted: {
            // Then scanner.ou (OptUri) is valid and we can use it to fill in the data.
            fillInput(imageScanner.value);
        }
    }

    QrCodeScanner {
        id: videoScanner

        onAccepted: {
            // Then scanner.ou (OptUri) is valid and we can use it to fill in the data.
            fillInput(videoScanner.value);
        }
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button cancel and dismiss the add account form", "Cancel")
            icon.name: "edit-undo"
            onTriggered: {
                root.vm.cancelled();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button Dismiss the error page and quit Keysmith", "Quit")
            icon.name: "application-exit"
            enabled: root.vm.quitEnabled
            visible: root.vm.quitEnabled
            onTriggered: {
                Qt.quit();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Scan from camera")
            icon.name: "view-barcode-qr-symbolic"
            onTriggered: {
                videoScanner.open();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Scan from image")
            icon.name: "insert-image-symbolic"
            onTriggered: {
                imageScanner.open();
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Add")
            icon.name: "answer-correct"
            enabled: root.acceptable
            onTriggered: {
                root.vm.accepted();
            }
        }
    ]

    Component.onCompleted: accountName.forceActiveFocus()

    AccountNameForm {
        id: accountName
        Layout.fillWidth: true
        accounts: root.vm.accounts
        validateAccountAvailability: root.vm.validateAvailability
        validatedInput: root.vm.input
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
            checked: root.vm.input.type === Models.ValidatedAccountInput.Totp
            text: i18nc("@option:radio", "Time-based OTP")
            onCheckedChanged: {
                if (checked) {
                    root.vm.input.type = Models.ValidatedAccountInput.Totp;
                }
            }
        }

        FormCard.FormRadioDelegate {
            id: hotpRadio
            checked: root.vm.input.type === Models.ValidatedAccountInput.Hotp
            text: i18nc("@option:radio", "Hash-based OTP")
            onCheckedChanged: {
                if (checked) {
                    root.vm.input.type = Models.ValidatedAccountInput.Hotp;
                }
            }
        }

        FormCard.FormDelegateSeparator { below: hotpRadio }

        FormCard.FormTextFieldDelegate {
            id: accountSecret
            placeholderText: i18n("Token secret")
            text: root.vm.input.secret
            echoMode: TextInput.Password
            label: i18nc("@label:textbox", "Secret key:")
            validator: Validators.Base32SecretValidator {
                id: secretValidator
            }
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
            onTextChanged: {
                if (acceptableInput) {
                    root.vm.input.secret = text;
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
        enabled: root.detailsEnabled && root.vm.input.type === Models.ValidatedAccountInput.Hotp
    }

    FormCard.FormHeader {
        title: i18nc("@label", "TOTP Details:")
        visible: totpDetails.visible
    }

    TOTPDetailsForm {
        id: totpDetails

        visible: enabled
        validatedInput: root.vm.input
        enabled: root.detailsEnabled && root.vm.input.type === Models.ValidatedAccountInput.Totp
    }
}
