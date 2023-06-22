/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import Keysmith.Application 1.0 as Application
import Keysmith.Models 1.0 as Models
import Keysmith.Validators 1.0 as Validators

Kirigami.ScrollablePage {
    id: root

    required property Application.AddAccountViewModel vm

    title: i18nc("@title:window", "Add new account")

    property bool detailsEnabled: false

    property bool secretAcceptable: accountSecret.acceptableInput
    property bool tokenTypeAcceptable: hotpRadio.checked || totpRadio.checked
    property bool hotpDetailsAcceptable: hotpDetails.acceptable || vm.input.type === Models.ValidatedAccountInput.Totp
    property bool totpDetailsAcceptable: totpDetails.acceptable || vm.input.type === Models.ValidatedAccountInput.Hotp
    property bool tokenDetailsAcceptable: hotpDetailsAcceptable && totpDetailsAcceptable
    property bool acceptable: accountName.acceptable && secretAcceptable && tokenTypeAcceptable && tokenDetailsAcceptable

    leftPadding: 0
    rightPadding: 0

    Connections {
        target: vm.input
        function onTypeChanged() {
            root.detailsEnabled = false;
        }
    }

    Component.onCompleted: accountName.forceActiveFocus()

    ColumnLayout {
        spacing: 0
        AccountNameForm {
            id: accountName
            Layout.fillWidth: true
            accounts: vm.accounts
            validateAccountAvailability: vm.validateAvailability
            validatedInput: vm.input
        }

        MobileForm.FormCard {
            id: requiredDetails
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            contentItem: ColumnLayout {
                Layout.rowSpan: 2

                MobileForm.FormCardHeader {
                    title: i18nc("@label:chooser", "Account type:")
                }

                MobileForm.FormRadioDelegate {
                    id: totpRadio
                    checked: vm.input.type === Models.ValidatedAccountInput.Totp
                    text: i18nc("@option:radio", "Time-based OTP")
                    onCheckedChanged: {
                        if (checked) {
                            vm.input.type = Models.ValidatedAccountInput.Totp;
                        }
                    }
                }

                MobileForm.FormRadioDelegate {
                    id: hotpRadio
                    checked: vm.input.type === Models.ValidatedAccountInput.Hotp
                    text: i18nc("@option:radio", "Hash-based OTP")
                    onCheckedChanged: {
                        if (checked) {
                            vm.input.type = Models.ValidatedAccountInput.Hotp;
                        }
                    }
                }

                MobileForm.FormDelegateSeparator { below: hotpRadio }

                MobileForm.FormTextFieldDelegate {
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

                MobileForm.FormDelegateSeparator { above: toggleDetails }

                MobileForm.FormButtonDelegate {
                    id: toggleDetails
                    text: i18nc("Button to reveal form for configuring additional token details", "Details")
                    enabled: !root.detailsEnabled && (hotpRadio.checked || totpRadio.checked)
                    visible: enabled
                    onClicked: root.detailsEnabled = true;
                }
            }
        }

        HOTPDetailsForm {
            id: hotpDetails

            visible: enabled
            validatedInput: root.vm.input
            enabled: root.detailsEnabled && vm.input.type === Models.ValidatedAccountInput.Hotp

            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
        }

        TOTPDetailsForm {
            id: totpDetails

            visible: enabled
            validatedInput: root.vm.input
            enabled: root.detailsEnabled && vm.input.type === Models.ValidatedAccountInput.Totp

            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
        }
    }

    actions.left: Kirigami.Action {
        text: i18nc("@action:button cancel and dismiss the add account form", "Cancel")
        iconName: "edit-undo"
        onTriggered: {
            vm.cancelled();
        }
    }
    actions.right: Kirigami.Action {
        text: i18nc("@action:button Dismiss the error page and quit Keysmtih", "Quit")
        iconName: "application-exit"
        enabled: vm.quitEnabled
        visible: vm.quitEnabled
        onTriggered: {
            Qt.quit();
        }
    }
    actions.main: Kirigami.Action {
        text: i18n("Add")
        iconName: "answer-correct"
        enabled: acceptable
        onTriggered: {
            vm.accepted();
        }
    }
}
