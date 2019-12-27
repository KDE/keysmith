/*
 * Copyright 2019 Bhushan Shah <bshah@kde.org>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License or any later version accepted by the membership of 
 * KDE e.V. (or its successor approved by the membership of KDE
 * e.V.), which shall act as a proxy defined in Section 14 of
 * version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "app/keysmith.h"
#include "model/accounts.h"
#include "../validators/qmlsupport.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("keysmith");

    QCoreApplication::setOrganizationName("KDE");
    QCoreApplication::setOrganizationDomain("kde.org");
    QCoreApplication::setApplicationName("Keysmith");
    QGuiApplication::setApplicationDisplayName(i18nc("@title", "Keysmith"));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    validators::registerValidatorTypes();
    qmlRegisterUncreatableType<model::SimpleAccountListModel>("Keysmith.Models", 1, 0, "AccountListModel", "Use the Keysmith singleton to obtain an AccountListModel");
    qmlRegisterUncreatableType<model::AccountView>("Keysmith.Models", 1, 0, "Account", "Use an AccountListModel from the Keysmith singleton to obtain an Account");
    qmlRegisterSingletonType<app::Keysmith>("Keysmith.Application", 1, 0, "Keysmith", [](QQmlEngine *qml, QJSEngine *js) -> QObject *
    {
        Q_UNUSED(qml);
        Q_UNUSED(js);

        return new app::Keysmith();
    });

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    int ret = app.exec();
    return ret;
}
