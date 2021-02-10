/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2019-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import Keysmith.Application 1.0
import Keysmith.Models 1.0 as Models

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    property bool addActionEnabled: !addAccountRequested
    property bool addAccountAvailable: false
    property bool addAccountConfirmed: false
    property bool addAccountRequested: CommandLine.newAccountRequested
    property Models.AccountListModel accounts: Keysmith.accountListModel()
    property Models.ValidatedAccountInput validatedInput: Models.ValidatedAccountInput {}
    property Models.PasswordRequestModel passwordRequest: Keysmith.passwordRequest()

    Kirigami.PageRouter {
        id: router
        initialRoute: "__init__"
        pageStack: root.pageStack.columnView

        // FIXME: dummy just to have a valid initialRoute
        Kirigami.PageRoute {
            name: "__init__"
            Component {
                Kirigami.Page {

                }
            }
        }

        Kirigami.PageRoute {
            name: "setup"
            Component {
                SetupPassword {
                }
            }
        }

        Kirigami.PageRoute {
            name: "unlock"
            Component {
                UnlockAccounts {
                }
            }
        }

        Kirigami.PageRoute {
            name: "accounts"
            Component {
                AccountsOverview {
                    accounts: root.accounts
                    addActionEnabled: root.addActionEnabled
                    onAccountWanted: {
                        Kirigami.PageRouter.pushRoute("add-new");
                        root.addActionEnabled = false;
                    }
                }
            }
        }

        Kirigami.PageRoute {
            name: "add-new"
            Component {
                AddAccount {
                    accounts: root.accounts
                    onCancelled: {
                        root.addActionEnabled = true;
                        Kirigami.PageRouter.navigateToRoute("accounts");
                    }
                    onNewAccount: {
                        root.addActionEnabled = true;
                        root.accounts.addAccount(input);
                        Kirigami.PageRouter.navigateToRoute("accounts");
                    }
                }
            }
        }

        Kirigami.PageRoute {
            name: "accept-pushed"
            Component {
                AddAccount {
                    quitEnabled: true
                    validateAccountAvailability: false
                    validatedInput: root.validatedInput
                    accounts: root.accounts
                    onQuit: {
                        Qt.quit();
                    }
                    onCancelled: {
                        root.addAccountConfirmed = false;
                        root.addActionEnabled = true;
                        pushPasswordPage();
                    }
                    onNewAccount: {
                        root.addAccountConfirmed = true;
                        root.addActionEnabled = true;
                        pushPasswordPage();
                    }
                }
            }
        }

        Kirigami.PageRoute {
            name: "rename-pushed"
            Component {
                RenameAccount {
                    accounts: root.accounts
                    validatedInput: root.validatedInput
                    onCancelled: {
                        root.addActionEnabled = true;
                        Kirigami.PageRouter.navigateToRoute("accounts");
                    }
                    onNewAccount: {
                        root.addActionEnabled = true;
                        root.accounts.addAccount(root.validatedInput);
                        Kirigami.PageRouter.navigateToRoute("accounts");
                    }
                }
            }
        }

        Kirigami.PageRoute {
            name: "invalid-uri"
            Component {
                ErrorPage {
                    quitEnabled: true
                    title: i18nc("@title:window", "Invalid account")
                    error: i18nc("@info:label", "The account you are trying to add is invalid. You can either quit the app, or continue without adding the account.")
                    onDismissed: {
                        pushPasswordPage();
                    }
                    onQuit: {
                        Qt.quit();
                    }
                }
            }
        }
    }

    function autoAddNewAccountFromCommandLine() {
        // accounts not yet loaded, keep the user waiting...
        if (!accounts.loaded) {
            return;
        }

        if (!passwordRequest.keyAvailable) {
            return; // TODO warn if not
        }

        // nothing to do (anymore)
        if (!root.addAccountConfirmed) {
            return;
        }

        // claim the new account, try to see if it can be added directly or needs renaming/recovery
        addAccountConfirmed = false;
        if(accounts.isAccountStillAvailable(validatedInput.name, validatedInput.issuer)) {
            accounts.addAccount(validatedInput);
            addActionEnabled = true;
            router.navigateToRoute("accounts");
        } else {
            router.navigateToRoute("rename-pushed");
        }
    }

    function pushPasswordPage() {
        /*
         * Sanity check that the password request has been resolved already.
         * If it is not yet clear which type of password request is needed, then signals should still
         * fire once this becomes clear and the request is fully resolved.
         */
        if (passwordRequest.previouslyDefined || passwordRequest.firstRun) {
            router.navigateToRoute(passwordRequest.previouslyDefined ? "unlock" : "setup");
        }
    }

    /*
     * TODO maybe have a onPasswordProvided handler to push a "progress" page to provide visual feedback for devices
     * where key derivation is slow?
     */
    Connections {
        target: passwordRequest
        onPasswordAccepted : {
            // TODO convert to C++ helper, have proper logging?
            if (!passwordRequest.keyAvailable) {
                return; // TODO warn if not
            }

            if (root.addAccountConfirmed) {
                autoAddNewAccountFromCommandLine();
            } else {
                root.addActionEnabled = true;
                router.navigateToRoute("accounts");
            }
        }
        onPasswordExists: {
            /*
             * Ignore if there is an account from the commandline to process: in that case password unlocking is
             * deferred until after account confirmation/rejection by the user
             */
            if (!addAccountRequested) {
                pushPasswordPage();
            }
        }
        onNewPasswordNeeded: {
            /*
             * Ignore if there is an account from the commandline to process: in that case password setup is deferred
             * until after account confirmation/rejection by the user
             */
            if (!addAccountRequested) {
                pushPasswordPage();
            }
        }
    }

    Connections {
        target: accounts
        onLoadedChanged: {
            if (accounts.loaded) {
                autoAddNewAccountFromCommandLine();
            }
        }
    }

    Connections {
        target: CommandLine

        onNewAccountInvalid: {
            // TODO properly report error in UX

            if (addAccountRequested) {
                addAccountAvailable = false;
                addAccountConfirmed = false;
                addActionEnabled = true;
                router.navigateToRoute("invalid-uri");
            }
            // TODO warn if not
        }

        onNewAccountProcessed: {
            addAccountAvailable = true;
            router.navigateToRoute("accept-pushed");
        }
    }

    Component.onCompleted: {
        if (addAccountRequested) {
            root.validatedInput.reset();
            CommandLine.handleNewAccount(root.validatedInput);
        } else {
            /*
             * In case password request resolves more quickly than the UI, make sure to check if it has been resolved
             * yet and prompt for passwords.
             */
            pushPasswordPage();
        }
    }
}
