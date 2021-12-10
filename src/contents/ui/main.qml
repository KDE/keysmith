/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2019-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import org.kde.kirigami 2.12 as Kirigami
import QtQml 2.15

import Keysmith.Application 1.0 as Application

Kirigami.ApplicationWindow {
    id: root

    width: Kirigami.Units.gridUnit * 28
    height: Kirigami.Units.gridUnit * 28

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
            name: Application.Keysmith.navigation.name(Application.Navigation.SetupPassword)
            Component {
                SetupPassword {
                    vm: Kirigami.PageRouter.data
                }
            }
        }

        Kirigami.PageRoute {
            name: Application.Keysmith.navigation.name(Application.Navigation.UnlockAccounts)
            Component {
                UnlockAccounts {
                    vm: Kirigami.PageRouter.data
                }
            }
        }

        Kirigami.PageRoute {
            name: Application.Keysmith.navigation.name(Application.Navigation.AccountsOverview)
            Component {
                AccountsOverview {
                    vm: Kirigami.PageRouter.data
                }
            }
        }

        Kirigami.PageRoute {
            name: Application.Keysmith.navigation.name(Application.Navigation.AddAccount)
            Component {
                AddAccount {
                    vm: Kirigami.PageRouter.data
                }
            }
        }

        Kirigami.PageRoute {
            name: Application.Keysmith.navigation.name(Application.Navigation.RenameAccount)
            Component {
                RenameAccount {
                    vm: Kirigami.PageRouter.data
                }
            }
        }

        Kirigami.PageRoute {
            name: Application.Keysmith.navigation.name(Application.Navigation.ErrorPage)
            Component {
                ErrorPage {
                    vm: Kirigami.PageRouter.data
                }
            }
        }
    }

    Connections {
        target: Application.Keysmith.navigation
        function onRouted (route, data) {
            router.navigateToRoute({route, data});
        }
        function onPushed(route, data) {
            router.pushRoute({route, data});
        }
    }
}
