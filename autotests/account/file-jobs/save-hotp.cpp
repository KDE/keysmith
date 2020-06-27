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

#include <limits>

Q_DECLARE_METATYPE(std::optional<uint>);

class SaveHotpTest: public QObject
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
    QTest::addColumn<QString>("issuer");
    QTest::addColumn<quint64>("counter");
    QTest::addColumn<int>("tokenLength");
    QTest::addColumn<std::optional<uint>>("offset");
    QTest::addColumn<bool>("checksum");
    QTest::addColumn<QString>("actualAccountsIni");
    QTest::addColumn<QString>("expectedAccountsIni");
}

static void define_test_case(const char * label, const QUuid &id, const QString &accountName, const QString &issuer, quint64 counter, int tokenLength, const std::optional<uint> &offset, bool checksum, const QString &actualIni, const QString &expectedIni)
{
    QTest::newRow(label) << id << accountName << issuer << counter << tokenLength << offset << checksum << actualIni << expectedIni;
}

void SaveHotpTest::validHotp(void)
{
    QFETCH(QUuid, id);
    QFETCH(QString, name);
    QFETCH(QString, issuer);
    QFETCH(quint64, counter);
    QFETCH(int, tokenLength);
    QFETCH(std::optional<uint>, offset);
    QFETCH(bool, checksum);
    QFETCH(QString, actualAccountsIni);
    QFETCH(QString, expectedAccountsIni);

    const QString actual = test::path(actualAccountsIni);
    const QString lock = test::path(actualAccountsIni + QLatin1String(".lock"));
    bool actionRun = false;

    const accounts::SettingsProvider settings([&actual, &actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actual, QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    std::optional<secrets::EncryptedSecret> tokenSecret = test::encrypt(&m_secret, QByteArray("Hello, world!"));
    QVERIFY2(tokenSecret, "should be able to encrypt the token secret");

    accounts::SaveHotp uut(settings, id, name, issuer, *tokenSecret, counter, tokenLength, offset, checksum);
    QSignalSpy invalidAccount(&uut, &accounts::SaveHotp::invalid);
    QSignalSpy savedAccount(&uut, &accounts::SaveHotp::saved);
    QSignalSpy jobFinished(&uut, &accounts::SaveHotp::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(savedAccount), "account should be saved");
    QVERIFY2(actionRun, "accounts action should have run");

    QFile result(actual);
    QVERIFY2(result.exists(), "accounts file should have been created");
    QCOMPARE(test::slurp(actual), test::slurp(expectedAccountsIni));

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
    QFETCH(QString, issuer);
    QFETCH(quint64, counter);
    QFETCH(int, tokenLength);
    QFETCH(std::optional<uint>, offset);
    QFETCH(bool, checksum);
    QFETCH(QString, actualAccountsIni);
    QFETCH(QString, expectedAccountsIni);

    const QString actual = test::path(actualAccountsIni);
    const QString lock = test::path(actualAccountsIni + QLatin1String(".lock"));
    bool actionRun = false;

    const accounts::SettingsProvider settings([&actual, &actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actual, QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    std::optional<secrets::EncryptedSecret> tokenSecret = test::encrypt(&m_secret, QByteArray("Hello, world!"));
    QVERIFY2(tokenSecret, "should be able to encrypt the token secret");

    accounts::SaveHotp uut(settings, id, name, issuer, *tokenSecret, counter, tokenLength, offset, checksum);
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
    define_test_case("valid-hotp-sample-1", QUuid("072a645d-6c26-57cc-81eb-d9ef3b9b39e2"), QLatin1String("valid-hotp-sample-1"), QString(), 6U,
                     0U, std::nullopt, false,
                     QLatin1String("save-valid-hotp-accounts-1.ini"), QLatin1String(":/save-hotp/expected-accounts-1.ini"));
    define_test_case("valid-hotp-sample-2", QUuid("437c23aa-2fb0-519a-9a34-a5a2671eea24"), QLatin1String("valid-hotp-sample-2"), QLatin1String("autotests"), 6U,
                     0U, std::optional<uint>(12U), true,
                     QLatin1String("save-valid-hotp-accounts-2.ini"), QLatin1String(":/save-hotp/expected-accounts-2.ini"));
}

void SaveHotpTest::invalidHotp_data(void)
{
    define_test_data();
    define_test_case("null UUID", QUuid(), QLatin1String("null UUID"), QString(), 6U,
                     0U, std::nullopt, false,
                     QLatin1String("save-hotp-dummy-accounts-1.ini"), QString());
    define_test_case("null account name", QUuid("00611bbf-5e0b-5c6a-9847-ad865315ce86"), QString(), QString(), 6U,
                     0U, std::nullopt, false,
                     QLatin1String("save-hotp-dummy-accounts-2.ini"), QString());
    define_test_case("empty account name", QUuid("1e42b907-99d8-5da3-a59b-89b257e49c83"), QLatin1String(""), QString(), 6U,
                     0U, std::nullopt, false,
                     QLatin1String("save-hotp-dummy-accounts-3.ini"), QString());
    define_test_case("empty issuer name", QUuid("533b406b-ad04-5203-a26f-5deb0afeba22"), QLatin1String("empty issuer name"), QLatin1String(""), 6U,
                     0U, std::nullopt, false,
                     QLatin1String("save-hotp-dummy-accounts-4.ini"), QString());
    define_test_case("invalid issuer name", QUuid("1c1ffa42-bb9f-5413-a8a7-6c5b0eb8a36f"), QLatin1String("invalid issuer name"), QLatin1String(":"), 6U,
                     0U, std::nullopt, false,
                     QLatin1String("save-hotp-dummy-accounts-5.ini"), QString());
    define_test_case("tokenLength too small", QUuid("bca12e13-4b5b-5e4e-b162-3b86a6284dea"), QLatin1String("tokenLength too small"), QString(), 5U,
                     0U, std::nullopt, false,
                     QLatin1String("save-hotp-dummy-accounts-6.ini"), QString());
    define_test_case("tokenLength too large", QUuid("5c10d530-fb22-5438-848d-3d4d1f738610"), QLatin1String("tokenLength too large"), QString(), 11U,
                     0U, std::nullopt, false,
                     QLatin1String("save-hotp-dummy-accounts-7.ini"), QString());
    define_test_case("offset too large", QUuid("d0f545da-cc1e-57aa-b793-d856757f33e8"), QLatin1String("offset too large"), QString(), 6U,
                     0U, std::optional<uint>(17U), false,
                     QLatin1String("save-hotp-dummy-accounts-8.ini"), QString());
    define_test_case("offset out of range", QUuid("b31acaca-ee8f-54aa-948e-d67789fbe74c"), QLatin1String("offset out of range"), QString(), 6U,
                     0U, std::optional<uint>(std::numeric_limits<uint>::max()), false,
                     QLatin1String("save-hotp-dummy-accounts-9.ini"), QString());
}

void SaveHotpTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::useDummyPassword(&m_secret), "should be able to set up the master key");
}

QTEST_MAIN(SaveHotpTest)

#include "save-hotp.moc"
