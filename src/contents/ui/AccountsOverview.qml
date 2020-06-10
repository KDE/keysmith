/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.10 as Kirigami

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models

Kirigami.ScrollablePage {
    id: root
    /*
     * Explicitly opt-out of scroll-to-refresh/drag-to-refresh behaviour
     * Underlying model implementations don't offer the hooks for that.
     */
    supportsRefreshing: false
    title: i18nc("@title:window", "Accounts")

    signal accountWanted
    property bool addActionEnabled : true
    property Models.AccountListModel accounts: Keysmith.accountListModel()

    property string accountErrorMessage: i18nc("generic error shown when adding or updating an account failed", "Failed to update accounts")
    property string loadingErrorMessage: i18nc("error message shown when loading accounts from storage failed", "Some accounts failed to load.")
    property string errorMessage: loadingErrorMessage

    header: ColumnLayout {
        id: column
        Layout.margins: 0
        spacing: 0
        Kirigami.InlineMessage {
            id: message
            visible: root.accounts.error
            type: Kirigami.MessageType.Error
            text: root.errorMessage
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
                    root.accounts.error = false;
                }
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: root.accounts.error
        }
    }

    Component {
        id: hotpDelegate
        HOTPAccountEntryView {
            account: value
            onActionTriggered: {
                root.accounts.error = false;
                root.errorMessage = root.accountErrorMessage;
            }
        }
    }

    Component {
        id: totpDelegate
        TOTPAccountEntryView {
            account: value
            onActionTriggered: {
                root.accounts.error = false;
                root.errorMessage = root.accountErrorMessage;
            }
        }
    }

    ListView {
        id: mainList
        model: Models.SortedAccountListModel {
            sourceModel: accounts
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
                text: section
            }
        }
    }

    actions.main: Kirigami.Action {
        id: addAction
        text: i18n("Add")
        iconName: "list-add"
        visible: addActionEnabled
        onTriggered: {
            root.accounts.error = false;
            root.errorMessage = root.accountErrorMessage;
            root.accountWanted();
        }
    }
}
