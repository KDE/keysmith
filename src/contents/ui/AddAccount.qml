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

Kirigami.Page {
    id: root
    title: i18nc("@title:window", "Add new account")
    signal dismissed
    property Models.AccountListModel accounts: Keysmith.accountListModel()
    property bool secretAcceptable: accountSecret.acceptableInput
    property bool tokenTypeAcceptable: hotpRadio.checked || totpRadio.checked
    property bool acceptable: accountName.acceptable && secretAcceptable && tokenTypeAcceptable && tokenDetails.acceptable

    property Models.ValidatedAccountInput validatedInput: Models.ValidatedAccountInput {
    }

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        AccountNameForm {
            id: accountName
            validatedInput: root.validatedInput
        }
        Kirigami.FormLayout {
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
        }
        TokenDetailsForm {
            id: tokenDetails
            validatedInput: root.validatedInput
        }
    }

    actions.main: Kirigami.Action {
        text: i18n("Add")
        iconName: "answer-correct"
        enabled: acceptable
        onTriggered: {
            root.accounts.addAccount(root.validatedInput);
            root.dismissed();
        }
    }
}
