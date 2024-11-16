/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1 as FormCard

import Keysmith.Application as Application
import Keysmith.Models as Models
import Keysmith.Validators as Validators

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Import Accounts")

    required property Application.ImportAccountViewModel vm

    property bool passwordRequired: false

    readonly property bool formatComboboxAcceptable: formatCombobox.currentIndex !== -1
    readonly property bool accountsFileAcceptable: accountsFile.selectedFile.toString() !== ""
    readonly property bool passwordAcceptable: !passwordRequired || password.text !== ""
    readonly property bool acceptable: formatComboboxAcceptable && accountsFileAcceptable && passwordAcceptable

    topPadding: Kirigami.Units.gridUnit
    bottomPadding: Kirigami.Units.gridUnit

    data: Connections {
        target: vm.input
        function onFormatChanged() {
            root.passwordRequired = [
                Models.ValidatedImportInput.AndOTPEncryptedJSON,
                Models.ValidatedImportInput.AegisEncryptedJSON
            ].includes(formatCombobox.currentValue);
        }
    }

    Component.onCompleted: formatCombobox.forceActiveFocus()

    FormCard.FormCard {
        id: requiredDetails

        FormCard.FormComboBoxDelegate {
            id: formatCombobox
            text: i18n("Import format")

            model: ListModel {
                Component.onCompleted: {
                    // ListModel doesn't support i18n strings
                    append({name: i18nc("@item:inlistbox", "andOTP Encrypted JSON"), value: Models.ValidatedImportInput.AndOTPEncryptedJSON});
                    append({name: i18nc("@item:inlistbox", "andOTP Plain JSON"), value: Models.ValidatedImportInput.AndOTPPlainJSON});
                    //append({name: i18nc("@item:inlistbox", "Aegis Encrypted JSON"), value: Models.ValidatedImportInput.AegisEncryptedJSON});
                    append({name: i18nc("@item:inlistbox", "Aegis Plain JSON"), value: Models.ValidatedImportInput.AegisPlainJSON});
                    append({name: i18nc("@item:inlistbox", "FreeOTP URIs"), value: Models.ValidatedImportInput.FreeOTPURIs});

                    formatCombobox.currentIndex = 0;
                }
            }

            textRole: "name"
            valueRole: "value"

            onCurrentValueChanged: vm.input.format = currentIndex;
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            id: openFileDialog
            text: i18nc("@label:chooser", "Backup file:")
            enabled: formatComboboxAcceptable
            onClicked: accountsFile.open();
            description: vm.input.file.toString().length > 0 ? vm.input.file.toString().substring(7) : i18nc("@info:placeholder", "No file selected")

            Dialogs.FileDialog {
                id: accountsFile
                title: i18nc("@title:window", "Select file")
                onAccepted: {
                    vm.input.file = accountsFile.selectedFile;
                }
            }
        }

        FormCard.FormDelegateSeparator {
            visible: root.passwordRequired
        }

        FormCard.FormTextFieldDelegate {
            id: password
            visible: root.passwordRequired
            placeholderText: i18nc("@info:placeholder", "Decryption password")
            text: vm.input.password
            echoMode: TextInput.Password
            label: i18nc("@label:textbox", "Password:")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
            onTextChanged: vm.input.password = text;
            onAccepted: vm.accepted();
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.gridUnit

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Import")
            icon.name: "answer-correct-symbolic"
            enabled: acceptable
            onClicked: {
                vm.accepted();
            }
        }
    }
}
