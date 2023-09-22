/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/output.h"
#include "../test-utils/secret.h"
#include "../../test-utils/spy.h"
#include "../../secrets/test-utils/random.h"

#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUuid>
#include <QtDebug>

static QString emptyIniResource(QLatin1String("empty-accounts.ini"));
static QString corpusIniResource(QLatin1String("sample-accounts.ini"));
static QString invalidIniResource(QLatin1String("invalid-accounts.ini"));

class LoadAccountsTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void emptyAccountsFile(void);
    void sampleAccountsFile(void);
    void invalidSampleAccountsFile(void);
private:
    accounts::AccountSecret m_secret {&test::fakeRandom};
};

static QByteArray rawSecret = QByteArray::fromBase64(QByteArray("8juE9gJFLp3OgL4CxJ5v5q8sw+h7Vbn06+NY4uc="), QByteArray::Base64Encoding);
static QByteArray rawNonce = QByteArray::fromBase64(QByteArray("QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFB"), QByteArray::Base64Encoding);

static qint64 dummyClock(void)
{
    return 1'234'567'890LL;
}

void LoadAccountsTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::copyResource(QStringLiteral(":/load-accounts/empty-accounts.ini"), emptyIniResource), "empty INI resource should be available as file");
    QVERIFY2(test::copyResource(QStringLiteral(":/load-accounts/sample-accounts.ini"), corpusIniResource), "test corpus INI resource should be available as file");
    QVERIFY2(test::copyResource(QStringLiteral(":/load-accounts/invalid-accounts.ini"), invalidIniResource), "invalid INI resource should be available as file");
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

    accounts::LoadAccounts uut(settings, &m_secret, &dummyClock);

    QSignalSpy hotpFound(&uut, &accounts::LoadAccounts::foundHotp);
    QSignalSpy totpFound(&uut, &accounts::LoadAccounts::foundTotp);
    QSignalSpy loadingError(&uut, &accounts::LoadAccounts::failedToLoadAllAccounts);
    QSignalSpy jobFinished(&uut, &accounts::LoadAccounts::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QVERIFY2(actionRun, "accounts action should have run");
    QCOMPARE(hotpFound.count(), 0);
    QCOMPARE(totpFound.count(), 0);
    QCOMPARE(loadingError.count(), 0);
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

    accounts::LoadAccounts uut(settings, &m_secret, &dummyClock);

    QSignalSpy hotpFound(&uut, &accounts::LoadAccounts::foundHotp);
    QSignalSpy totpFound(&uut, &accounts::LoadAccounts::foundTotp);
    QSignalSpy loadingError(&uut, &accounts::LoadAccounts::failedToLoadAllAccounts);
    QSignalSpy jobFinished(&uut, &accounts::LoadAccounts::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QVERIFY2(actionRun, "accounts action should have run");
    QCOMPARE(hotpFound.count(), 2);
    QCOMPARE(totpFound.count(), 2);
    QCOMPARE(loadingError.count(), 0);

    const auto firstHotp = hotpFound.at(0);
    QCOMPARE(firstHotp.at(0).toUuid(), QUuid(QLatin1String("072a645d-6c26-57cc-81eb-d9ef3b9b39e2")));
    QCOMPARE(firstHotp.at(1).toString(), QLatin1String("valid-hotp-sample-1"));
    QCOMPARE(firstHotp.at(2).toString(), QString());
    QCOMPARE(firstHotp.at(3).toByteArray(), rawSecret);
    QCOMPARE(firstHotp.at(4).toByteArray(), rawNonce);
    QCOMPARE(firstHotp.at(5).toUInt(), 6U);
    QCOMPARE(firstHotp.at(6).toULongLong(), 0ULL);
    QCOMPARE(firstHotp.at(7).toBool(), false);
    QCOMPARE(firstHotp.at(8).toUInt(), 0U);
    QCOMPARE(firstHotp.at(9).toBool(), false);

    const auto secondHotp = hotpFound.at(1);
    QCOMPARE(secondHotp.at(0).toUuid(), QUuid(QLatin1String("437c23aa-2fb0-519a-9a34-a5a2671eea24")));
    QCOMPARE(secondHotp.at(1).toString(), QLatin1String("valid-hotp-sample-2"));
    QCOMPARE(secondHotp.at(2).toString(), QLatin1String("autotests"));
    QCOMPARE(secondHotp.at(3).toByteArray(), rawSecret);
    QCOMPARE(secondHotp.at(4).toByteArray(), rawNonce);
    QCOMPARE(secondHotp.at(5).toUInt(), 6U);
    QCOMPARE(secondHotp.at(6).toULongLong(), 0ULL);
    QCOMPARE(secondHotp.at(7).toBool(), true);
    QCOMPARE(secondHotp.at(8).toUInt(), 12U);
    QCOMPARE(secondHotp.at(9).toBool(), true);

    const auto firstTotp = totpFound.at(0);
    QCOMPARE(firstTotp.at(0).toUuid(), QUuid(QLatin1String("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc")));
    QCOMPARE(firstTotp.at(1).toString(), QLatin1String("valid-totp-sample-1"));
    QCOMPARE(firstTotp.at(2).toString(), QString());
    QCOMPARE(firstHotp.at(3).toByteArray(), rawSecret);
    QCOMPARE(firstHotp.at(4).toByteArray(), rawNonce);
    QCOMPARE(firstTotp.at(5).toUInt(), 6);
    QCOMPARE(firstTotp.at(6).toUInt(), 30U);
    QCOMPARE(firstTotp.at(7).toDateTime(), QDateTime::fromMSecsSinceEpoch(0));
    QCOMPARE(firstTotp.at(8).value<accounts::Account::Hash>(), accounts::Account::Hash::Sha1);

    const auto secondTotp = totpFound.at(1);
    QCOMPARE(secondTotp.at(0).toUuid(), QUuid(QLatin1String("6537d6a5-005e-5a92-b560-b09df3c2e676")));
    QCOMPARE(secondTotp.at(1).toString(), QLatin1String("valid-totp-sample-2"));
    QCOMPARE(secondTotp.at(2).toString(), QLatin1String("autotests"));
    QCOMPARE(secondHotp.at(3).toByteArray(), rawSecret);
    QCOMPARE(secondHotp.at(4).toByteArray(), rawNonce);
    QCOMPARE(secondTotp.at(5).toUInt(), 6U);
    QCOMPARE(secondTotp.at(6).toUInt(), 30U);
    QCOMPARE(secondTotp.at(7).toDateTime(), QDateTime::fromMSecsSinceEpoch(1'234'567'890LL).toUTC());
    QCOMPARE(secondTotp.at(8).value<accounts::Account::Hash>(), accounts::Account::Hash::Sha256);
}

void LoadAccountsTest::invalidSampleAccountsFile(void)
{
    bool actionRun = false;
    const accounts::SettingsProvider settings([&actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(test::path(invalidIniResource), QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    accounts::LoadAccounts uut(settings, &m_secret, &dummyClock);

    QSignalSpy hotpFound(&uut, &accounts::LoadAccounts::foundHotp);
    QSignalSpy totpFound(&uut, &accounts::LoadAccounts::foundTotp);
    QSignalSpy loadingError(&uut, &accounts::LoadAccounts::failedToLoadAllAccounts);
    QSignalSpy jobFinished(&uut, &accounts::LoadAccounts::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QVERIFY2(actionRun, "accounts action should have run");
    QCOMPARE(hotpFound.count(), 0);
    QCOMPARE(totpFound.count(), 0);
    QCOMPARE(loadingError.count(), 1);
}

QTEST_MAIN(LoadAccountsTest)

#include "load-accounts.moc"
