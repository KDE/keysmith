/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2019-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

import QtQml 2.15
import org.kde.kirigami 2.20 as Kirigami

import Keysmith.Application 1.0 as Application

Kirigami.ApplicationWindow {
    id: root

    width: Kirigami.Units.gridUnit * 20
    height: Kirigami.Units.gridUnit * 30

    function routeToUrl(route) {
        switch (route) {
        case Application.Navigation.Error:
            return "qrc:/ErrorPage.qml"
        case Application.Navigation.UnlockAccounts:
            return "qrc:/UnlockAccounts.qml"
        case Application.Navigation.RenameAccount:
            return "qrc:/RenameAccount.qml"
        case Application.Navigation.AccountsOverview:
            return "qrc:/AccountsOverview.qml"
        case Application.Navigation.AddAccount:
            return "qrc:/AddAccount.qml"
        case Application.Navigation.SetupPassword:
            return "qrc:/SetupPassword.qml"
        }
        return 'bug';
    }

    Connections {
        target: Application.Keysmith.navigation

        function onRouted(route, data) {
            console.log(route, data)
            const pageUrl = routeToUrl(route);
            while (root.pageStack.depth > 1) {
                root.pageStack.pop();
            }
            if (root.pageStack.depth === 0) {
                root.pageStack.push(pageUrl, {
                    vm: data,
                });
            } else {
                root.pageStack.replace(pageUrl, {
                    vm: data,
                });
            }
        }

        function onPushed(route, data) {
            const pageUrl = routeToUrl(route);
            root.pageStack.push(pageUrl, {
                vm: data,
            });
        }
    }
}
