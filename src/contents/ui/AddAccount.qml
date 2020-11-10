/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.8 as Kirigami

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models
import Keysmith.Validators 1.0 as Validators

Kirigami.ScrollablePage {
    id: root
    title: i18nc("@title:window", "Add new account")
    signal quit
    signal cancelled
    signal newAccount(var input)
    property Models.AccountListModel accounts: Keysmith.accountListModel()
    property bool quitEnabled: false
    property bool detailsEnabled: false
    property bool validateAccountAvailability: true

    property bool secretAcceptable: accountSecret.acceptableInput
    property bool tokenTypeAcceptable: hotpRadio.checked || totpRadio.checked
    property bool hotpDetailsAcceptable: hotpDetails.acceptable || validatedInput.type === Models.ValidatedAccountInput.Totp
    property bool totpDetailsAcceptable: totpDetails.acceptable || validatedInput.type === Models.ValidatedAccountInput.Hotp
    property bool tokenDetailsAcceptable: hotpDetailsAcceptable && totpDetailsAcceptable
    property bool acceptable: accountName.acceptable && secretAcceptable && tokenTypeAcceptable && tokenDetailsAcceptable

    property Models.ValidatedAccountInput validatedInput: Models.ValidatedAccountInput {}

    Connections {
        target: validatedInput
        onTypeChanged: {
            root.detailsEnabled = false;
        }
    }

    ColumnLayout {
        spacing: 0
        AccountNameForm {
            id: accountName
            Layout.fillWidth: true
            validateAccountAvailability: root.validateAccountAvailability
            validatedInput: root.validatedInput
            twinFormLayouts: [requiredDetails, hotpDetails, totpDetails]
        }

        Kirigami.FormLayout {
            id: requiredDetails
            Layout.fillWidth: true
            twinFormLayouts: [accountName, hotpDetails, totpDetails]
            ColumnLayout {
                Layout.rowSpan: 2
                Kirigami.FormData.label: i18nc("@label:chooser", "Account Type:")
                Kirigami.FormData.buddyFor: totpRadio
                Controls.RadioButton {
                    id: totpRadio
                    checked: validatedInput.type === Models.ValidatedAccountInput.Totp
                    text: i18nc("@option:radio", "Time-based OTP")
                    onCheckedChanged: {
                        if (checked) {
                            validatedInput.type = Models.ValidatedAccountInput.Totp;
                        }
                    }
                }
                Controls.RadioButton {
                    id: hotpRadio
                    checked: validatedInput.type === Models.ValidatedAccountInput.Hotp
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
                text: validatedInput.secret
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

            Controls.Button {
                id: toggleDetails
                text: i18nc("Button to reveal form for configuring additional token details", "Details")
                enabled: !root.detailsEnabled && (hotpRadio.checked || totpRadio.checked)
                visible: enabled
                onClicked: {
                    root.detailsEnabled = true;
                }
            }
        }
        HOTPDetailsForm {
            visible: enabled
            id: hotpDetails
            Layout.fillWidth: true
            validatedInput: root.validatedInput
            twinFormLayouts: [accountName, requiredDetails, totpDetails]
            enabled: root.detailsEnabled && validatedInput.type === Models.ValidatedAccountInput.Hotp
        }
        TOTPDetailsForm {
            visible: enabled
            id: totpDetails
            Layout.fillWidth: true
            validatedInput: root.validatedInput
            twinFormLayouts: [accountName, requiredDetails, hotpDetails]
            enabled: root.detailsEnabled && validatedInput.type === Models.ValidatedAccountInput.Totp
        }
    }

    actions.left: Kirigami.Action {
        text: i18nc("@action:button cancel and dismiss the add account form", "Cancel")
        iconName: "edit-undo"
        onTriggered: {
            root.cancelled();
        }
    }
    actions.right: Kirigami.Action {
        text: i18nc("@action:button Dismiss the error page and quit Keysmtih", "Quit")
        iconName: "application-exit"
        enabled: root.quitEnabled
        visible: root.quitEnabled
        onTriggered: {
            root.quit();
        }
    }
    actions.main: Kirigami.Action {
        text: i18n("Add")
        iconName: "answer-correct"
        enabled: acceptable
        onTriggered: {
            root.newAccount(root.validatedInput);
        }
    }
}
