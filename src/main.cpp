/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "app/keysmith.h"
#include "model/accounts.h"
#include "validators/countervalidator.h"
#include "validators/secretvalidator.h"

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

    qmlRegisterUncreatableType<model::SimpleAccountListModel>("Keysmith.Models", 1, 0, "AccountListModel", "Use the Keysmith singleton to obtain an AccountListModel");
    qmlRegisterUncreatableType<model::AccountView>("Keysmith.Models", 1, 0, "Account", "Use an AccountListModel from the Keysmith singleton to obtain an Account");
    qmlRegisterType<model::AccountNameValidator>("Keysmith.Validators", 1, 0, "AccountNameValidator");
    qmlRegisterType<validators::Base32Validator>("Keysmith.Validators", 1, 0, "Base32SecretValidator");
    qmlRegisterType<validators::UnsignedLongValidator>("Keysmith.Validators", 1, 0, "HOTPCounterValidator");
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
