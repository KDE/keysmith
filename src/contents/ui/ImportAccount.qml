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

    required property Application.ImportAccountViewModel vm

    title: i18nc("@title:window", "Import accounts")

    property bool passwordRequired: false

    property bool formatComboboxAcceptable: formatCombobox.currentIndex !== -1
    property bool accountsFileAcceptable: accountsFile.fileUrl.toString() !== ""
    property bool passwordAcceptable: !passwordRequired || password.text !== ""
    property bool acceptable: formatComboboxAcceptable && accountsFileAcceptable && passwordAcceptable

    topPadding: Kirigami.Units.gridUnit
    bottomPadding: Kirigami.Units.gridUnit

    data: Connections {
        target: vm.input
        function onFormatChanged() {
            root.passwordRequired = [
                Models.ValidatedImportInput.AndOTPEncryptedJSON,
                Models.ValidatedImportInput.AegisEncryptedJSON
            ].includes(formatCombobox.currentIndex);
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
            text: i18nc("@action:button", "Add")
            icon.name: "answer-correct"
            enabled: acceptable
            onTriggered: {
                vm.accepted();
            }
        }
    ]

    Component.onCompleted: formatCombobox.forceActiveFocus()

    FormCard.FormCard {
        id: requiredDetails

        FormCard.FormComboBoxDelegate {
            id: formatCombobox
            text: i18n("Import format")

            model: ListModel {
                Component.onCompleted: {
                    // ListModel doesn't support i18n strings
                    append({"name": i18nc("@item:inlistbox", "FreeOTP URIs"), "value": Models.ValidatedImportInput.FreeOTPURIs});
                    append({"name": i18nc("@item:inlistbox", "andOTP Plain JSON"), "value": Models.ValidatedImportInput.AndOTPPlainJSON});
                    append({"name": i18nc("@item:inlistbox", "andOTP Encrypted JSON"), "value": Models.ValidatedImportInput.AndOTPEncryptedJSON});
                    append({"name": i18nc("@item:inlistbox", "Aegis Plain JSON"), "value": Models.ValidatedImportInput.AegisPlainJSON});
                    append({"name": i18nc("@item:inlistbox", "Aegis Encrypted JSON"), "value": Models.ValidatedImportInput.AegisEncryptedJSON});

                    formatCombobox.currentIndex = formatCombobox.indexOfValue(Models.ValidatedImportInput.format);
                }
            }

            textRole: "name"
            valueRole: "value"

            onCurrentValueChanged: vm.input.format = currentIndex;
        }

        FormCard.FormButtonDelegate {
            id: openFileDialog
            text: i18nc("Button to choose file", "Open")
            enabled: formatComboboxAcceptable
            onClicked: accountsFile.open();

            Dialogs.FileDialog {
                id: accountsFile
                title: i18nc("@title:window", "Select file")
                onAccepted: vm.input.file = accountsFile.fileUrl;
            }
        }

        FormCard.FormTextFieldDelegate {
            id: password
            visible: enabled
            enabled: root.passwordRequired
            placeholderText: i18n("Decryption password")
            text: vm.input.password
            echoMode: TextInput.Password
            label: i18nc("@label:textbox", "Password")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhHiddenText
            onTextChanged: vm.input.password = text;
        }
    }
}
