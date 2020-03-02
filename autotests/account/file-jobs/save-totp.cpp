/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/output.h"
#include "../test-utils/secret.h"
#include "../test-utils/spy.h"

#include "../../secrets/test-utils/random.h"

#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUuid>
#include <QtDebug>

class SaveTotpTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void validHotp(void);
    void validHotp_data(void);
    void invalidHotp(void);
    void invalidHotp_data(void);
private:
    accounts::AccountSecret m_secret {&test::fakeRandom};
};

static void define_test_data(void)
{
    QTest::addColumn<QUuid>("id");
    QTest::addColumn<QString>("name");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<int>("tokenLength");
}

static void define_test_case(const char * label, const QUuid &id, const QString &accountName, uint timeStep, int tokenLength)
{
    QTest::newRow(label) << id << accountName << timeStep << tokenLength;
}

void SaveTotpTest::validHotp(void)
{
    QFETCH(QUuid, id);
    QFETCH(QString, name);
    QFETCH(uint, timeStep);
    QFETCH(int, tokenLength);

    const QString actual = test::path("actual-accounts.ini");
    const QString lock = test::path("actual-accounts.ini.lock");
    bool actionRun = false;

    const accounts::SettingsProvider settings([&actual, &actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actual, QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    std::optional<secrets::EncryptedSecret> tokenSecret = test::encrypt(&m_secret, QByteArray("Hello, world!"));
    QVERIFY2(tokenSecret, "should be able to encrypt the token secret");

    accounts::SaveTotp uut(settings, id, name, *tokenSecret, timeStep, tokenLength);
    QSignalSpy invalidAccount(&uut, &accounts::SaveTotp::invalid);
    QSignalSpy savedAccount(&uut, &accounts::SaveTotp::saved);
    QSignalSpy jobFinished(&uut, &accounts::SaveTotp::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(savedAccount), "account should be saved");
    QVERIFY2(actionRun, "accounts action should have run");

    QFile result(actual);
    QVERIFY2(result.exists(), "accounts file should have been created");
    QCOMPARE(test::slurp(actual), test::slurp(":/save-totp/expected-accounts.ini"));

    QFile lockFile(lock);
    QVERIFY2(!lockFile.exists(), "lock file should no longer exist");

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QCOMPARE(invalidAccount.count(), 0);
    QCOMPARE(savedAccount.count(), 1);
}

void SaveTotpTest::invalidHotp(void)
{
    QFETCH(QUuid, id);
    QFETCH(QString, name);
    QFETCH(uint, timeStep);
    QFETCH(int, tokenLength);

    const QString actual = test::path("dummy-accounts.ini");
    const QString lock = test::path("dummy-accounts.ini.lock");
    bool actionRun = false;

    const accounts::SettingsProvider settings([&actual, &actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actual, QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    std::optional<secrets::EncryptedSecret> tokenSecret = test::encrypt(&m_secret, QByteArray("Hello, world!"));
    QVERIFY2(tokenSecret, "should be able to encrypt the token secret");

    accounts::SaveTotp uut(settings, id, name, *tokenSecret, timeStep, tokenLength);
    QSignalSpy invalidAccount(&uut, &accounts::SaveTotp::invalid);
    QSignalSpy savedAccount(&uut, &accounts::SaveTotp::saved);
    QSignalSpy jobFinished(&uut, &accounts::SaveTotp::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QCOMPARE(invalidAccount.count(), 1);
    QVERIFY2(!actionRun, "accounts action should not have run");
    QCOMPARE(savedAccount.count(), 0);

    QFile result(actual);
    QVERIFY2(!result.exists(), "accounts file should not have been created");

    QFile lockFile(lock);
    QVERIFY2(!lockFile.exists(), "lock file should not have been created");
}

void SaveTotpTest::validHotp_data(void)
{
    define_test_data();
    define_test_case("valid-totp-sample-1", QUuid("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc"), QLatin1String("valid-totp-sample-1"), 30, 6);
}

void SaveTotpTest::invalidHotp_data(void)
{
    define_test_data();
    define_test_case("null UUID", QUuid(), QLatin1String("null UUID"), 30, 6);
    define_test_case("null account name", QUuid("00611bbf-5e0b-5c6a-9847-ad865315ce86"), QString(), 30, 6);
    define_test_case("empty account name", QUuid("1e42b907-99d8-5da3-a59b-89b257e49c83"), QLatin1String(""), 30, 6);
    define_test_case("timeStep too small", QUuid("5ab8749b-f973-5f48-a70e-c261ebd0521a"), QLatin1String("timeStep too small"), 0, 6);
    define_test_case("tokenLength too small", QUuid("bca12e13-4b5b-5e4e-b162-3b86a6284dea"), QLatin1String("tokenLength too small"), 30, 5);
    define_test_case("tokenLength too large", QUuid("5c10d530-fb22-5438-848d-3d4d1f738610"), QLatin1String("tokenLength too large"), 30, 11);
}

void SaveTotpTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::useDummyPassword(&m_secret), "should be able to set up the master key");
}

QTEST_MAIN(SaveTotpTest)

#include "save-totp.moc"
