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
    void validTotp(void);
    void validTotp_data(void);
    void invalidTotp(void);
    void invalidTotp_data(void);
private:
    accounts::AccountSecret m_secret {&test::fakeRandom};
};

static void define_test_data(void)
{
    QTest::addColumn<QUuid>("id");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("issuer");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<int>("tokenLength");
    QTest::addColumn<QDateTime>("epoch");
    QTest::addColumn<accounts::Account::Hash>("hash");
    QTest::addColumn<qint64>("now");
    QTest::addColumn<QString>("actualAccountsIni");
    QTest::addColumn<QString>("expectedAccountsIni");
}

static void define_test_case(const char * label, const QUuid &id, const QString &accountName, const QString &issuer, uint timeStep, int tokenLength, const QDateTime &epoch, accounts::Account::Hash hash, qint64 now, const QString &actualIni, const QString &expectedIni)
{
    QTest::newRow(label) << id << accountName << issuer << timeStep << tokenLength << epoch << hash << now << actualIni << expectedIni;
}

void SaveTotpTest::validTotp(void)
{
    QFETCH(QUuid, id);
    QFETCH(QString, name);
    QFETCH(QString, issuer);
    QFETCH(uint, timeStep);
    QFETCH(int, tokenLength);
    QFETCH(QDateTime, epoch);
    QFETCH(accounts::Account::Hash, hash);
    QFETCH(qint64, now);
    QFETCH(QString, actualAccountsIni);
    QFETCH(QString, expectedAccountsIni);

    const std::function<qint64(void)> clock([now](void) -> qint64
    {
        return now;
    });

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

    accounts::SaveTotp uut(settings, id, name, issuer, *tokenSecret, timeStep, tokenLength, epoch, hash, clock);
    QSignalSpy invalidAccount(&uut, &accounts::SaveTotp::invalid);
    QSignalSpy savedAccount(&uut, &accounts::SaveTotp::saved);
    QSignalSpy jobFinished(&uut, &accounts::SaveTotp::finished);

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

void SaveTotpTest::invalidTotp(void)
{
    QFETCH(QUuid, id);
    QFETCH(QString, name);
    QFETCH(QString, issuer);
    QFETCH(uint, timeStep);
    QFETCH(int, tokenLength);
    QFETCH(QDateTime, epoch);
    QFETCH(accounts::Account::Hash, hash);
    QFETCH(qint64, now);
    QFETCH(QString, actualAccountsIni);
    QFETCH(QString, expectedAccountsIni);

    const std::function<qint64(void)> clock([now](void) -> qint64
    {
        return now;
    });

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

    accounts::SaveTotp uut(settings, id, name, issuer, *tokenSecret, timeStep, tokenLength, epoch, hash, clock);
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

void SaveTotpTest::validTotp_data(void)
{
    define_test_data();
    define_test_case("valid-totp-sample-1", QUuid("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc"), QLatin1String("valid-totp-sample-1"), QString(), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(0), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-valid-totp-accounts-1.ini"), QLatin1String(":/save-totp/expected-accounts-1.ini"));
    define_test_case("valid-totp-sample-2", QUuid("6537d6a5-005e-5a92-b560-b09df3c2e676"), QLatin1String("valid-totp-sample-2"), QLatin1String("autotests"), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(1'234'567'890LL), accounts::Account::Hash::Sha512,
                     2'000'000'000LL, QLatin1String("save-valid-totp-accounts-2.ini"), QLatin1String(":/save-totp/expected-accounts-2.ini"));
}

void SaveTotpTest::invalidTotp_data(void)
{
    define_test_data();
    define_test_case("null UUID", QUuid(), QLatin1String("null UUID"), QString(), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-1.ini"), QString());
    define_test_case("null account name", QUuid("00611bbf-5e0b-5c6a-9847-ad865315ce86"), QString(), QString(), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-2.ini"), QString());
    define_test_case("empty account name", QUuid("1e42b907-99d8-5da3-a59b-89b257e49c83"), QLatin1String(""), QString(), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-3.ini"), QString());
    define_test_case("empty issuer name", QUuid("533b406b-ad04-5203-a26f-5deb0afeba22"), QLatin1String("empty issuer name"), QLatin1String(""), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-4.ini"), QString());
    define_test_case("empty issuer name", QUuid("1c1ffa42-bb9f-5413-a8a7-6c5b0eb8a36f"), QLatin1String("invalid issuer name"), QLatin1String(":"), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-5.ini"), QString());
    define_test_case("timeStep too small", QUuid("5ab8749b-f973-5f48-a70e-c261ebd0521a"), QLatin1String("timeStep too small"), QString(), 6U,
                     0U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-6.ini"), QString());
    define_test_case("tokenLength too small", QUuid("bca12e13-4b5b-5e4e-b162-3b86a6284dea"), QLatin1String("tokenLength too small"), QString(), 5U,
                     30U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-7.ini"), QString());
    define_test_case("tokenLength too large", QUuid("5c10d530-fb22-5438-848d-3d4d1f738610"), QLatin1String("tokenLength too large"), QString(), 11U,
                     30U, QDateTime::fromMSecsSinceEpoch(0LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-8.ini"), QString());
    define_test_case("null/invalid epoch datetime", QUuid("e719ed90-e0c0-5510-81c1-ccfd7a5e962c"), QLatin1String("null/invalid epoch datetime"), QString(), 6U,
                     30U, QDateTime(), accounts::Account::Hash::Sha1,
                     2'000'000'000LL, QLatin1String("save-totp-dummy-accounts-9.ini"), QString());
    define_test_case("future epoch datetime", QUuid("0e03a2ed-8e49-54ac-8e46-20536078e5a1"), QLatin1String("future epoch datetime"), QString(), 6U,
                     30U, QDateTime::fromMSecsSinceEpoch(1'234'567'890LL), accounts::Account::Hash::Sha1,
                     0LL, QLatin1String("save-totp-dummy-accounts-10.ini"), QString());
}

void SaveTotpTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::useDummyPassword(&m_secret), "should be able to set up the master key");
}

QTEST_MAIN(SaveTotpTest)

#include "save-totp.moc"
