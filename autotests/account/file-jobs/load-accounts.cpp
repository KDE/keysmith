/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/output.h"
#include "../test-utils/secret.h"
#include "../test-utils/spy.h"

#include "../../secrets/test-utils/random.h"

#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUuid>
#include <QtDebug>

static QString emptyIniResource(QLatin1String("empty-accounts.ini"));
static QString corpusIniResource(QLatin1String("sample-accounts.ini"));

class LoadAccountsTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void emptyAccountsFile(void);
    void sampleAccountsFile(void);
private:
    accounts::AccountSecret m_secret {&test::fakeRandom};
};

static QByteArray rawSecret = QByteArray::fromBase64(QByteArray("8juE9gJFLp3OgL4CxJ5v5q8sw+h7Vbn06+NY4uc="), QByteArray::Base64Encoding);
static QByteArray rawNonce = QByteArray::fromBase64(QByteArray("QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFB"), QByteArray::Base64Encoding);

void LoadAccountsTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::copyResource(":/load-accounts/empty-accounts.ini", emptyIniResource), "empty INI resource should be available as file");
    QVERIFY2(test::copyResource(":/load-accounts/sample-accounts.ini", corpusIniResource), "test corpus INI resource should be available as file");
    QVERIFY2(test::useDummyPassword(&m_secret), "should be able to set up the master key");
}

void LoadAccountsTest::emptyAccountsFile(void)
{
    bool actionRun = false;
    const accounts::SettingsProvider settings([&actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(test::path(emptyIniResource), QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    accounts::LoadAccounts uut(settings, &m_secret);

    QSignalSpy hotpFound(&uut, &accounts::LoadAccounts::foundHotp);
    QSignalSpy totpFound(&uut, &accounts::LoadAccounts::foundTotp);
    QSignalSpy jobFinished(&uut, &accounts::LoadAccounts::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QVERIFY2(actionRun, "accounts action should have run");
    QCOMPARE(hotpFound.count(), 0);
    QCOMPARE(totpFound.count(), 0);
}

void LoadAccountsTest::sampleAccountsFile(void)
{
    bool actionRun = false;
    const accounts::SettingsProvider settings([&actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(test::path(corpusIniResource), QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    accounts::LoadAccounts uut(settings, &m_secret);

    QSignalSpy hotpFound(&uut, &accounts::LoadAccounts::foundHotp);
    QSignalSpy totpFound(&uut, &accounts::LoadAccounts::foundTotp);
    QSignalSpy jobFinished(&uut, &accounts::LoadAccounts::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QVERIFY2(actionRun, "accounts action should have run");
    QCOMPARE(hotpFound.count(), 1);
    QCOMPARE(totpFound.count(), 1);

    const auto firstHotp = hotpFound.at(0);
    QCOMPARE(firstHotp.at(0).toUuid(), QUuid(QLatin1String("072a645d-6c26-57cc-81eb-d9ef3b9b39e2")));
    QCOMPARE(firstHotp.at(1).toString(), QLatin1String("valid-hotp-sample-1"));
    QCOMPARE(firstHotp.at(2).toByteArray(), rawSecret);
    QCOMPARE(firstHotp.at(3).toByteArray(), rawNonce);
    QCOMPARE(firstHotp.at(4).toULongLong(), 0ULL);
    QCOMPARE(firstHotp.at(5).toInt(), 6);

    const auto firstTotp = totpFound.at(0);
    QCOMPARE(firstTotp.at(0).toUuid(), QUuid(QLatin1String("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc")));
    QCOMPARE(firstTotp.at(1).toString(), QLatin1String("valid-totp-sample-1"));
    QCOMPARE(firstHotp.at(2).toByteArray(), rawSecret);
    QCOMPARE(firstHotp.at(3).toByteArray(), rawNonce);
    QCOMPARE(firstTotp.at(4).toUInt(), 30);
    QCOMPARE(firstTotp.at(5).toInt(), 6);
}

QTEST_MAIN(LoadAccountsTest)

#include "load-accounts.moc"
