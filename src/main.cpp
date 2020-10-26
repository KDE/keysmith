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
#include "model/input.h"
#include "validators/countervalidator.h"
#include "validators/datetimevalidator.h"
#include "validators/issuervalidator.h"
#include "validators/secretvalidator.h"

/*
 * Integrate QML debugging/profiling support, conditional on building Keysmith in Debug mode.
 * NDEBUG is defined by the C standard and automatically set by CMake during a Release type build, hence the double negative.
 */
#ifndef NDEBUG
#include <QQmlDebuggingEnabler>
static QQmlDebuggingEnabler enabler;
#endif

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("keysmith");

    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(QStringLiteral("Keysmith"));
    QGuiApplication::setApplicationDisplayName(i18nc("@title", "Keysmith"));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    qmlRegisterUncreatableType<model::SimpleAccountListModel>("Keysmith.Models", 1, 0, "AccountListModel",
        QStringLiteral("Use the Keysmith singleton to obtain an AccountListModel")
    );
    qmlRegisterUncreatableType<model::PasswordRequest>("Keysmith.Models", 1, 0, "PasswordRequestModel",
        QStringLiteral("Use the Keysmith singleton to obtain an PasswordRequestModel")
    );
    qmlRegisterUncreatableType<model::AccountView>("Keysmith.Models", 1, 0, "Account",
        QStringLiteral("Use an AccountListModel from the Keysmith singleton to obtain an Account")
    );
    qmlRegisterType<model::AccountInput>("Keysmith.Models", 1, 0, "ValidatedAccountInput");
    qmlRegisterType<model::SortedAccountsListModel>("Keysmith.Models", 1, 0, "SortedAccountListModel");
    qmlRegisterType<model::AccountNameValidator>("Keysmith.Validators", 1, 0, "AccountNameValidator");
    qmlRegisterType<validators::EpochValidator>("Keysmith.Validators", 1, 0, "TOTPEpochValidator");
    qmlRegisterType<validators::IssuerValidator>("Keysmith.Validators", 1, 0, "AccountIssuerValidator");
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
