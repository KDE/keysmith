/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "app/cli.h"

#include "../test-utils/spy.h"

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <QtDebug>

class CommandLineOptionsTest: public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void testValidAccountUri(void);
    void testInvalidAccountUri(void);
    void testInvalidCommandLine(void);
};

void CommandLineOptionsTest::initTestCase(void)
{
    auto threadPool = QThreadPool::globalInstance();
    QVERIFY2(threadPool, "should have a global thread pool by now");
    threadPool->setMaxThreadCount(3);
}

static bool prime(QCommandLineParser &parser, const QStringList &argv)
{
    app::CommandLineOptions::addOptions(parser);
    return parser.parse(argv);
}

void CommandLineOptionsTest::testValidAccountUri(void)
{
    QCommandLineParser parser;
    model::AccountInput recipient;
    const auto argv = QStringList()
        << QStringLiteral("<dummy app>")
        << QStringLiteral("otpauth://hotp/issuer:valid?secret=VALUE&digits=8&period=60&issuer=issuer&counter=42&algorithm=sha512");

    app::CommandLineOptions uut(parser, prime(parser, argv));
    QSignalSpy invalid(&uut, &app::CommandLineOptions::newAccountInvalid);
    QSignalSpy processed(&uut, &app::CommandLineOptions::newAccountProcessed);

    QVERIFY2(uut.newAccountRequested(), "Account URI parsing should be requested");
    QVERIFY2(uut.optionsOk(), "Commandline options should be marked as 'ok' (valid)");
    QCOMPARE(uut.errorText(), QString());

    uut.handleNewAccount(&recipient);
    QVERIFY2(test::signal_eventually_emitted_once(processed), "Account URI should be processed by now");
    QCOMPARE(invalid.count(), 0);

    QCOMPARE(recipient.name(), QStringLiteral("valid"));
    QCOMPARE(recipient.issuer(), QStringLiteral("issuer"));
    QCOMPARE(recipient.counter(), QStringLiteral("42"));
    QCOMPARE(recipient.secret(), QStringLiteral("VALUE==="));
    QCOMPARE(recipient.tokenLength(), 8U);
    QCOMPARE(recipient.timeStep(), 60U);
    QCOMPARE(recipient.type(), model::AccountInput::TokenType::Hotp);
    QCOMPARE(recipient.algorithm(), model::AccountInput::TOTPAlgorithm::Sha512);

    QVERIFY2(QThreadPool::globalInstance()->waitForDone(500), "the global thread pool should be done by now");
}

void CommandLineOptionsTest::testInvalidAccountUri(void)
{
    QCommandLineParser parser;
    model::AccountInput recipient;
    const auto argv = QStringList()
        << QStringLiteral("<dummy app>")
        << QStringLiteral("not a valid otpauth:// URI");

    app::CommandLineOptions uut(parser, prime(parser, argv));
    QSignalSpy invalid(&uut, &app::CommandLineOptions::newAccountInvalid);
    QSignalSpy processed(&uut, &app::CommandLineOptions::newAccountProcessed);

    QVERIFY2(uut.newAccountRequested(), "Account URI parsing should be requested");
    QVERIFY2(uut.optionsOk(), "Commandline options should be marked as 'ok' (valid)");
    QCOMPARE(uut.errorText(), QString());

    uut.handleNewAccount(&recipient);
    QVERIFY2(test::signal_eventually_emitted_once(invalid), "Account URI should have been rejected by now");
    QCOMPARE(processed.count(), 0);

    QCOMPARE(recipient.name(), QString());
    QCOMPARE(recipient.issuer(), QString());
    QCOMPARE(recipient.counter(), QStringLiteral("0"));
    QCOMPARE(recipient.secret(), QString());
    QCOMPARE(recipient.tokenLength(), 6U);
    QCOMPARE(recipient.timeStep(), 30U);
    QCOMPARE(recipient.type(), model::AccountInput::TokenType::Totp);
    QCOMPARE(recipient.algorithm(), model::AccountInput::TOTPAlgorithm::Sha1);

    QVERIFY2(QThreadPool::globalInstance()->waitForDone(500), "the global thread pool should be done by now");
}

void CommandLineOptionsTest::testInvalidCommandLine(void)
{
    QCommandLineParser parser;
    const auto argv = QStringList()
        << QStringLiteral("<dummy app>")
        << QStringLiteral("--invalid-option");

    app::CommandLineOptions uut(parser, prime(parser, argv));
    QSignalSpy invalid(&uut, &app::CommandLineOptions::newAccountInvalid);
    QSignalSpy processed(&uut, &app::CommandLineOptions::newAccountProcessed);

    QVERIFY2(!uut.optionsOk(), "Commandline options should not be marked as 'ok' (invalid)");
    QVERIFY2(!uut.errorText().isEmpty(), "Commandline error message should not be empty");
    QVERIFY2(!uut.newAccountRequested(), "Account URI parsing should not be requested");

    QCoreApplication::processEvents(QEventLoop::AllEvents, 500);
    QCOMPARE(processed.count(), 0);
    QCOMPARE(invalid.count(), 0);

    QVERIFY2(QThreadPool::globalInstance()->waitForDone(500), "the global thread pool should be done by now");
}

QTEST_GUILESS_MAIN(CommandLineOptionsTest)

#include "commandline-options.moc"
