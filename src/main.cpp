/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "app/cli.h"
#include "app/keysmith.h"
#include "app/vms.h"
#include "model/accounts.h"
#include "model/input.h"
#include "validators/countervalidator.h"
#include "validators/datetimevalidator.h"
#include "validators/issuervalidator.h"
#include "validators/secretvalidator.h"

#include "version.h"

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
    QCoreApplication::setApplicationVersion(KEYSMITH_VERSION_STRING);
    QGuiApplication::setApplicationDisplayName(i18nc("@title", "Keysmith"));

    QCommandLineParser cliParser;

    // default/boilerplate options handled entirely via command line
    const auto helpOption = cliParser.addHelpOption();
    const auto versionOption = cliParser.addVersionOption();

    bool parseOk = app::Proxy::parseCommandLine(cliParser, QCoreApplication::arguments());

    /*
     * First check for pure command line options and handle these.
     * If any are found, the application should not bother with an UI.
     */

    if (cliParser.isSet(helpOption)) {
        int ret = parseOk ? 0 : 1;
        cliParser.showHelp(ret);
        return ret; // just to be explicit: showHelp() is documented to call exit()
    }

    if (cliParser.isSet(versionOption)) {
        cliParser.showVersion();
        return 0; // just to be explicit: showVersion() is documented to call exit()
    }

    app::Proxy proxy(&app);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    qmlRegisterUncreatableType<app::AddAccountViewModel>("Keysmith.Application", 1, 0, "AddAccountViewModel",
        QStringLiteral("Should be automatically provided through Keysmith.Application.Navigation signals")
    );
    qmlRegisterUncreatableType<app::RenameAccountViewModel>("Keysmith.Application", 1, 0, "RenameAccountViewModel",
        QStringLiteral("Should be automatically provided through Keysmith.Navigation signals")
    );
    qmlRegisterUncreatableType<app::ErrorViewModel>("Keysmith.Application", 1, 0, "ErrorViewModel",
        QStringLiteral("Should be automatically provided through Keysmith.Navigation signals")
    );
    qmlRegisterUncreatableType<app::SetupPasswordViewModel>("Keysmith.Application", 1, 0, "SetupPasswordViewModel",
        QStringLiteral("Should be automatically provided through Keysmith.Navigation signals")
    );
    qmlRegisterUncreatableType<app::UnlockAccountsViewModel>("Keysmith.Application", 1, 0, "UnlockAccountsViewModel",
        QStringLiteral("Should be automatically provided through Keysmith.Navigation signals")
    );
    qmlRegisterUncreatableType<app::AccountsOverviewViewModel>("Keysmith.Application", 1, 0, "AccountsOverviewViewModel",
        QStringLiteral("Should be automatically provided through Keysmith.Navigation signals")
    );
    qmlRegisterUncreatableType<app::Navigation>("Keysmith.Application", 1, 0, "Navigation",
        QStringLiteral("Use the Keysmith singleton to obtain a Navigation")
    );

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
    qmlRegisterSingletonType<app::Keysmith>("Keysmith.Application", 1, 0, "Keysmith", [&proxy](QQmlEngine *qml, QJSEngine *js) -> QObject *
    {
        Q_UNUSED(js);

        auto app = new app::Keysmith(new app::Navigation(qml));
        proxy.enable(app);
        return app;
    });
    qmlRegisterSingletonType<app::CommandLineOptions>("Keysmith.Application", 1, 0, "CommandLine", [parseOk, &cliParser](QQmlEngine *qml, QJSEngine *js) -> QObject *
    {
        Q_UNUSED(qml);
        Q_UNUSED(js);

        return new app::CommandLineOptions(cliParser, parseOk);
    });

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    proxy.proxy(cliParser, parseOk);
    int ret = app.exec();
    return ret;
}
