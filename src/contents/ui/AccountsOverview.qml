/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Keysmith.Application as Application
import Keysmith.Models as Models

Kirigami.ScrollablePage {
    id: root
    /*
     * Explicitly opt-out of scroll-to-refresh/drag-to-refresh behaviour
     * Underlying model implementations don't offer the hooks for that.
     */
    supportsRefreshing: false
    title: i18nc("@title:window", "Accounts")

    property Application.AccountsOverviewViewModel vm

    property string accountErrorMessage: i18nc("generic error shown when adding or updating an account failed", "Failed to update accounts")
    property string loadingErrorMessage: i18nc("error message shown when loading accounts from storage failed", "Some accounts failed to load.")
    property string errorMessage: loadingErrorMessage

    header: ColumnLayout {
        id: column
        Layout.margins: 0
        spacing: 0
        Kirigami.InlineMessage {
            id: message
            visible: vm.accounts.error // FIXME : should be managed via vm
            type: Kirigami.MessageType.Error
            text: root.errorMessage // FIXME : should be managed via vm
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            /*
             * There is supposed to be a more Kirigami-way to allow the user to dismiss the error message: showCloseButton
             * Unfortunately:
             *
             *  - Kirigami doesn't really offer a direct API for detecting when the close button is clicked.
             *    Observing the close button's effect via the visible property works just as well, but it is a bit of a hack.
             *  - It results in a rather unattractive vertical sizing problem: the close button is rather big for inline text
             *    This makes the internal horizontal spacing look completely out of proportion with the vertical spacing.
             *  - The actual click/tap target is only a small fraction of the entire message (banner).
             *    In this case, making the entire message click/tap target would be much better.
             *
             * Solution: add a MouseArea for dismissing the message via click/tap.
             */
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    // FIXME : should be managed via vm
                    vm.accounts.error = false;
                }
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: vm.accounts.error // FIXME : should be managed via vm
        }
    }

    Component {
        id: hotpDelegate
        HOTPAccountEntryView {
            account: value
            onActionTriggered: {
                // FIXME : should be managed via vm
                vm.accounts.error = false;
                root.errorMessage = root.accountErrorMessage;
            }
        }
    }

    Component {
        id: totpDelegate
        TOTPAccountEntryView {
            account: value
            onActionTriggered: {
                // FIXME : should be managed via vm
                vm.accounts.error = false;
                root.errorMessage = root.accountErrorMessage;
            }
        }
    }

    ListView {
        id: mainList
        model: Models.SortedAccountListModel {
            sourceModel: vm.accounts
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: vm.accounts.loaded && mainList.count == 0
            text: i18n("No accounts added")
            icon.name: "unlock"

            helpfulAction: Kirigami.Action {
                icon.name: "list-add"
                text: i18nc("@action:button add new account, shown instead of overview list when no accounts have been added yet", "Add Account")
                onTriggered: {
                    // FIXME : should be managed via vm
                    vm.accounts.error = false;
                    root.errorMessage = root.accountErrorMessage;
                    vm.addNewAccount();
                }
            }
        }

        /*
         * Use a Loader to get a switch-like statement to select an
         * appropriate delegate based on properties of the account model.
         */
        delegate: Loader {
            /*
             * Fix up width manually.
             * It doesn't seem to be propagated correctly by itself otherwise.
             */
            width: mainList.width

            /*
             * The `model` and related properties from the ListView delegate
             * context will not survive into the actual delegate components.
             * However Loader will also inject properties in *its* context
             * which will be observed by those delegate components.
             *
             * Fill the Loader's context with properties from the model passed
             * by ListView, to make these values propagate into delegates.
             *
             * See also: https://doc.qt.io/qt-5/qml-qtquick-loader.html#using-a-loader-within-a-view-delegate
             */
            property Models.Account value: model.account
            property int index: model.index

            sourceComponent: {
                /*
                 * Guard against a broken account model.
                 * Will simply render nothing at all in that case.
                 */
                if (!value) {
                    // TODO warn about this
                    return null;
                }
                if (value.isHotp) {
                    return hotpDelegate;
                }
                if (value.isTotp) {
                    return totpDelegate;
                }

                // TODO warn about this
                return null;
            }
        }
        section {
            property: "account.issuer"
            delegate: Kirigami.ListSectionHeader {
                width: parent.width
                text: section
            }
        }
    }

    actions: [
        Kirigami.Action {
            id: addAction
            text: i18nc("@action:button add new account, shown in toolbar", "Add")
            icon.name: "list-add"
            enabled: vm.actionsEnabled
            visible: vm.actionsEnabled
            onTriggered: {
                // FIXME : should be managed via vm
                vm.accounts.error = false;
                root.errorMessage = root.accountErrorMessage;
                vm.addNewAccount();
            }
        },
        Kirigami.Action {
            id: importAction
            text: i18nc("@action:button import accounts, shown in toolbar", "Import")
            icon.name: "document-import"
            enabled: vm.actionsEnabled
            visible: vm.actionsEnabled
            onTriggered: {
                // FIXME : should be managed via vm
                vm.accounts.error = false;
                root.errorMessage = root.accountErrorMessage;
                vm.importAccount();
            }
        }
    ]
}
