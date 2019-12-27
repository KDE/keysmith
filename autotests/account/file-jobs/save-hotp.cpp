/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/output.h"
#include "../test-utils/spy.h"

#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUuid>
#include <QtDebug>

class SaveHotpTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void validHotp(void);
    void validHotp_data(void);
    void invalidHotp(void);
    void invalidHotp_data(void);
};

static void define_test_data(void)
{
    QTest::addColumn<QUuid>("id");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("secret");
    QTest::addColumn<quint64>("counter");
    QTest::addColumn<int>("tokenLength");
}

static void define_test_case(const char * label, const QUuid &id, const QString &accountName, const QString &secret, quint64 counter, int tokenLength)
{
    QTest::newRow(label) << id << accountName << secret << counter << tokenLength;
}

void SaveHotpTest::validHotp(void)
{
    QFETCH(QUuid, id);
    QFETCH(QString, name);
    QFETCH(QString, secret);
    QFETCH(quint64, counter);
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

    accounts::SaveHotp uut(settings, id, name, secret, counter, tokenLength);
    QSignalSpy invalidAccount(&uut, &accounts::SaveHotp::invalid);
    QSignalSpy savedAccount(&uut, &accounts::SaveHotp::saved);
    QSignalSpy jobFinished(&uut, &accounts::SaveHotp::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(savedAccount), "account should be saved");
    QVERIFY2(actionRun, "accounts action should have run");

    QFile result(actual);
    QVERIFY2(result.exists(), "accounts file should have been created");
    QCOMPARE(test::slurp(actual), test::slurp(":/save-hotp/expected-accounts.ini"));

    QFile lockFile(lock);
    QVERIFY2(!lockFile.exists(), "lock file should no longer exist");

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QCOMPARE(invalidAccount.count(), 0);
    QCOMPARE(savedAccount.count(), 1);
}

void SaveHotpTest::invalidHotp(void)
{
    QFETCH(QUuid, id);
    QFETCH(QString, name);
    QFETCH(QString, secret);
    QFETCH(quint64, counter);
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

    accounts::SaveHotp uut(settings, id, name, secret, counter, tokenLength);
    QSignalSpy invalidAccount(&uut, &accounts::SaveHotp::invalid);
    QSignalSpy savedAccount(&uut, &accounts::SaveHotp::saved);
    QSignalSpy jobFinished(&uut, &accounts::SaveHotp::finished);

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

void SaveHotpTest::validHotp_data(void)
{
    define_test_data();
    define_test_case("valid-hotp-sample-1", QUuid("072a645d-6c26-57cc-81eb-d9ef3b9b39e2"), QLatin1String("valid-hotp-sample-1"), QLatin1String("NBSWY3DPFQQHO33SNRSCCCQ="), 0, 6);
}

void SaveHotpTest::invalidHotp_data(void)
{
    define_test_data();
    define_test_case("null UUID", QUuid(), QLatin1String("null UUID"), QLatin1String("NBSWY3DPFQQHO33SNRSCCCQ="), 0, 6);
    define_test_case("null account name", QUuid("00611bbf-5e0b-5c6a-9847-ad865315ce86"), QString(), QLatin1String("NBSWY3DPFQQHO33SNRSCCCQ="), 0, 6);
    define_test_case("empty account name", QUuid("1e42b907-99d8-5da3-a59b-89b257e49c83"), QLatin1String(""), QLatin1String("NBSWY3DPFQQHO33SNRSCCCQ="), 0, 6);
    define_test_case("null secret", QUuid("6e5ba95c-984d-538c-844e-f9edc1341bd2"), QLatin1String("null secret"), QString(), 0, 6);
    define_test_case("empty secret", QUuid("fe68a65e-287e-5dcd-909b-1837d7ab94ee"), QLatin1String("empty secret"), QLatin1String(""), 0, 6);
    define_test_case("tokenLength too small", QUuid("bca12e13-4b5b-5e4e-b162-3b86a6284dea"), QLatin1String("tokenLength too small"), QLatin1String("NBSWY3DPFQQHO33SNRSCCCQ="), 0, 5);
    define_test_case("tokenLength too large", QUuid("5c10d530-fb22-5438-848d-3d4d1f738610"), QLatin1String("tokenLength too large"), QLatin1String("NBSWY3DPFQQHO33SNRSCCCQ="), 0, 9);
}

void SaveHotpTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
}

QTEST_MAIN(SaveHotpTest)

#include "save-hotp.moc"
