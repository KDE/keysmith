/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "app/cli.h"

#include "../test-utils/spy.h"

#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <QtDebug>

class CommandLineAccountJobTest: public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void testValidAccountUri(void);
    void testInvalidAccountUri(void);
    void testExpiredRecipient(void);
};

void CommandLineAccountJobTest::initTestCase(void)
{
    auto threadPool = QThreadPool::globalInstance();
    QVERIFY2(threadPool, "should have a global thread pool by now");
    threadPool->setMaxThreadCount(3);
}

void CommandLineAccountJobTest::testValidAccountUri(void)
{
    model::AccountInput recipient;
    auto uut = new app::CommandLineAccountJob(&recipient);

    QSignalSpy invalid(uut, &app::CommandLineAccountJob::newAccountInvalid);
    QSignalSpy processed(uut, &app::CommandLineAccountJob::newAccountProcessed);
    QSignalSpy cleaned(uut, &QObject::destroyed);

    uut->run(QStringLiteral("otpauth://hotp/issuer:valid?secret=VALUE&digits=8&period=60&issuer=issuer&counter=42&algorithm=sha512"));

    QVERIFY2(test::signal_eventually_emitted_once(processed), "URI should be successfully processed by now");
    QVERIFY2(test::signal_eventually_emitted_once(cleaned), "AccountJob should be disposed of by now");
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

void CommandLineAccountJobTest::testExpiredRecipient(void)
{
    auto recipient = new model::AccountInput();
    QSignalSpy expired(recipient, &QObject::destroyed);

    auto uut = new app::CommandLineAccountJob(recipient);

    QSignalSpy invalid(uut, &app::CommandLineAccountJob::newAccountInvalid);
    QSignalSpy processed(uut, &app::CommandLineAccountJob::newAccountProcessed);
    QSignalSpy cleaned(uut, &QObject::destroyed);

    recipient->deleteLater();
    QVERIFY2(test::signal_eventually_emitted_once(expired), "AccountInput should have expired by now");

    uut->run(QStringLiteral("otpauth://hotp/issuer:valid?secret=VALUE&digits=8&period=60&issuer=issuer&counter=42&algorithm=sha512"));

    QVERIFY2(test::signal_eventually_emitted_once(cleaned), "AccountJob should be disposed of by now");
    QCOMPARE(processed.count(), 0);
    QCOMPARE(invalid.count(), 0);

    QVERIFY2(QThreadPool::globalInstance()->waitForDone(500), "the global thread pool should be done by now");
}

void CommandLineAccountJobTest::testInvalidAccountUri(void)
{
    model::AccountInput recipient;
    auto uut = new app::CommandLineAccountJob(&recipient);

    QSignalSpy invalid(uut, &app::CommandLineAccountJob::newAccountInvalid);
    QSignalSpy processed(uut, &app::CommandLineAccountJob::newAccountProcessed);
    QSignalSpy cleaned(uut, &QObject::destroyed);

    uut->run(QStringLiteral("not a valid otpauth:// URI"));

    QVERIFY2(test::signal_eventually_emitted_once(invalid), "URI should be rejected by now");
    QVERIFY2(test::signal_eventually_emitted_once(cleaned), "AccountJob should be disposed of by now");
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

QTEST_GUILESS_MAIN(CommandLineAccountJobTest)

#include "commandline-account-job.moc"
