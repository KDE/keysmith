/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/output.h"
#include "../../secrets/test-utils/random.h"
#include "../../test-utils/spy.h"

#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QtDebug>

static QString existingPasswordIniResource(QLatin1String(":/request-account-password/existing-password.ini"));
static QString newPasswordIniResource(QLatin1String(":/request-account-password/new-password.ini"));
static QString newPasswordIniResultResource(QLatin1String(":/request-account-password/new-password-result.ini"));

class RequestAccountPasswordTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testExistingPassword(void);
    void testExistingPasswordAbort(void);
    void testNewPassword(void);
    void testNewPasswordAbort(void);
    void testAbortBeforeRun(void);
};

void RequestAccountPasswordTest::testAbortBeforeRun(void)
{
    const QString isolated(QLatin1String("abort-before-run.ini"));
    QVERIFY2(test::copyResourceAsWritable(newPasswordIniResource, isolated), "accounts INI resource should be available as file");

    int openCounter = 0;
    const QString actualIni = test::path(isolated);
    const accounts::SettingsProvider settings([&openCounter, &actualIni](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actualIni, QSettings::IniFormat);
        openCounter++;
        action(data);
    });

    accounts::AccountSecret secret;
    QSignalSpy existingPasswordNeeded(&secret, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&secret, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&secret, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy keyAvailable(&secret, &accounts::AccountSecret::keyAvailable);
    QSignalSpy passwordRequestsCancelled(&secret, &accounts::AccountSecret::requestsCancelled);

    accounts::RequestAccountPassword uut(settings, &secret);

    QSignalSpy failed(&uut, &accounts::RequestAccountPassword::failed);
    QSignalSpy unlocked(&uut, &accounts::RequestAccountPassword::unlocked);
    QSignalSpy jobFinished(&uut, &accounts::RequestAccountPassword::finished);

    secret.cancelRequests();
    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(passwordRequestsCancelled), "account secret should have signalled cancellation by now");
    QVERIFY2(test::signal_eventually_emitted_once(failed), "job should signal it failed to unlock the accounts");
    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");

    QCOMPARE(openCounter, 0);
    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(passwordRequestsCancelled.count(), 1);
    QCOMPARE(failed.count(), 1);
    QCOMPARE(unlocked.count(), 0);

    QFile result(actualIni);
    QVERIFY2(result.exists(), "accounts file should still exist");
    QCOMPARE(test::slurp(actualIni), test::slurp(newPasswordIniResource));
}

void RequestAccountPasswordTest::testNewPassword(void)
{
    const QString isolated(QLatin1String("supply-new-password.ini"));
    QVERIFY2(test::copyResourceAsWritable(newPasswordIniResource, isolated), "accounts INI resource should be available as file");

    int openCounter = 0;
    const QString actualIni = test::path(isolated);
    const accounts::SettingsProvider settings([&openCounter, &actualIni](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actualIni, QSettings::IniFormat);
        openCounter++;
        action(data);
    });

    accounts::AccountSecret secret(&test::fakeRandom);
    QSignalSpy existingPasswordNeeded(&secret, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&secret, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&secret, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy keyAvailable(&secret, &accounts::AccountSecret::keyAvailable);
    QSignalSpy passwordRequestsCancelled(&secret, &accounts::AccountSecret::requestsCancelled);

    accounts::RequestAccountPassword uut(settings, &secret);

    QSignalSpy failed(&uut, &accounts::RequestAccountPassword::failed);
    QSignalSpy unlocked(&uut, &accounts::RequestAccountPassword::unlocked);
    QSignalSpy jobFinished(&uut, &accounts::RequestAccountPassword::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(newPasswordNeeded), "(new) password should be asked for");
    QCOMPARE(openCounter, 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(failed.count(), 0);
    QCOMPARE(unlocked.count(), 0);
    QCOMPARE(jobFinished.count(), 0);

    QString password(QLatin1String("hello, world"));
    std::optional<secrets::KeyDerivationParameters> defaults = secrets::KeyDerivationParameters::create();
    QVERIFY2(defaults, "should be able to construct default key derivation parameters");
    QVERIFY2(secret.answerNewPassword(password, *defaults), "should be able to answer (new) password");

    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "(new) password should be accepted");
    QVERIFY2(test::signal_eventually_emitted_once(keyAvailable), "key should be derived");
    QVERIFY2(test::signal_eventually_emitted_once(unlocked), "accounts should be unlocked");
    QCOMPARE(openCounter, 2);

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");

    QCOMPARE(openCounter, 2);
    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(keyAvailable.count(), 1);
    QCOMPARE(passwordRequestsCancelled.count(), 0);
    QCOMPARE(failed.count(), 0);
    QCOMPARE(unlocked.count(), 1);

    QFile result(actualIni);
    QVERIFY2(result.exists(), "accounts file should still exist");
    QCOMPARE(test::slurp(actualIni), test::slurp(newPasswordIniResultResource));
}

void RequestAccountPasswordTest::testNewPasswordAbort(void)
{
    const QString isolated(QLatin1String("abort-new-password.ini"));
    QVERIFY2(test::copyResourceAsWritable(newPasswordIniResource, isolated), "accounts INI resource should be available as file");

    int openCounter = 0;
    const QString actualIni = test::path(isolated);
    const accounts::SettingsProvider settings([&openCounter, &actualIni](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actualIni, QSettings::IniFormat);
        openCounter++;
        action(data);
    });

    accounts::AccountSecret secret(&test::fakeRandom);
    QSignalSpy existingPasswordNeeded(&secret, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&secret, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&secret, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy keyAvailable(&secret, &accounts::AccountSecret::keyAvailable);
    QSignalSpy passwordRequestsCancelled(&secret, &accounts::AccountSecret::requestsCancelled);

    accounts::RequestAccountPassword uut(settings, &secret);

    QSignalSpy failed(&uut, &accounts::RequestAccountPassword::failed);
    QSignalSpy unlocked(&uut, &accounts::RequestAccountPassword::unlocked);
    QSignalSpy jobFinished(&uut, &accounts::RequestAccountPassword::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(newPasswordNeeded), "(new) password should be asked for");
    QCOMPARE(openCounter, 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(failed.count(), 0);
    QCOMPARE(unlocked.count(), 0);
    QCOMPARE(jobFinished.count(), 0);

     secret.cancelRequests();

    QVERIFY2(test::signal_eventually_emitted_once(passwordRequestsCancelled), "account secret should have signalled cancellation by now");
    QVERIFY2(test::signal_eventually_emitted_once(failed), "job should signal it failed to unlock the accounts");
    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");

    QCOMPARE(openCounter, 1);
    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(passwordRequestsCancelled.count(), 1);
    QCOMPARE(failed.count(), 1);
    QCOMPARE(unlocked.count(), 0);

    QFile result(actualIni);
    QVERIFY2(result.exists(), "accounts file should still exist");
    QCOMPARE(test::slurp(actualIni), test::slurp(newPasswordIniResource));
}

void RequestAccountPasswordTest::testExistingPassword(void)
{
    const QString isolated(QLatin1String("supply-existing-password.ini"));
    QVERIFY2(test::copyResourceAsWritable(existingPasswordIniResource, isolated), "accounts INI resource should be available as file");

    int openCounter = 0;
    const QString actualIni = test::path(isolated);
    const accounts::SettingsProvider settings([&openCounter, &actualIni](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actualIni, QSettings::IniFormat);
        openCounter++;
        action(data);
    });

    accounts::AccountSecret secret;
    QSignalSpy existingPasswordNeeded(&secret, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&secret, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&secret, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy keyAvailable(&secret, &accounts::AccountSecret::keyAvailable);
    QSignalSpy passwordRequestsCancelled(&secret, &accounts::AccountSecret::requestsCancelled);

    accounts::RequestAccountPassword uut(settings, &secret);

    QSignalSpy failed(&uut, &accounts::RequestAccountPassword::failed);
    QSignalSpy unlocked(&uut, &accounts::RequestAccountPassword::unlocked);
    QSignalSpy jobFinished(&uut, &accounts::RequestAccountPassword::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "(existing) password should be asked for");
    QCOMPARE(openCounter, 1);
    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(failed.count(), 0);
    QCOMPARE(unlocked.count(), 0);
    QCOMPARE(jobFinished.count(), 0);

    QString password(QLatin1String("hello, world"));
    QVERIFY2(secret.answerExistingPassword(password), "should be able to answer (existing) password");

    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "(existing) password should be accepted");
    QVERIFY2(test::signal_eventually_emitted_once(keyAvailable), "key should be derived");
    QVERIFY2(test::signal_eventually_emitted_once(unlocked), "accounts should be unlocked");
    QCOMPARE(openCounter, 2);

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");

    QCOMPARE(openCounter, 2);
    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(keyAvailable.count(), 1);
    QCOMPARE(passwordRequestsCancelled.count(), 0);
    QCOMPARE(failed.count(), 0);
    QCOMPARE(unlocked.count(), 1);

    QFile result(actualIni);
    QVERIFY2(result.exists(), "accounts file should still exist");
    QCOMPARE(test::slurp(actualIni), test::slurp(existingPasswordIniResource));
}

void RequestAccountPasswordTest::testExistingPasswordAbort(void)
{
    const QString isolated(QLatin1String("abort-existing-password.ini"));
    QVERIFY2(test::copyResourceAsWritable(existingPasswordIniResource, isolated), "accounts INI resource should be available as file");

    int openCounter = 0;
    const QString actualIni = test::path(isolated);
    const accounts::SettingsProvider settings([&openCounter, &actualIni](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actualIni, QSettings::IniFormat);
        openCounter++;
        action(data);
    });

    accounts::AccountSecret secret;
    QSignalSpy existingPasswordNeeded(&secret, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&secret, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&secret, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy keyAvailable(&secret, &accounts::AccountSecret::keyAvailable);
    QSignalSpy passwordRequestsCancelled(&secret, &accounts::AccountSecret::requestsCancelled);

    accounts::RequestAccountPassword uut(settings, &secret);

    QSignalSpy failed(&uut, &accounts::RequestAccountPassword::failed);
    QSignalSpy unlocked(&uut, &accounts::RequestAccountPassword::unlocked);
    QSignalSpy jobFinished(&uut, &accounts::RequestAccountPassword::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "(existing) password should be asked for");
    QCOMPARE(openCounter, 1);
    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(failed.count(), 0);
    QCOMPARE(unlocked.count(), 0);
    QCOMPARE(jobFinished.count(), 0);

    secret.cancelRequests();

    QVERIFY2(test::signal_eventually_emitted_once(passwordRequestsCancelled), "account secret should have signalled cancellation by now");
    QVERIFY2(test::signal_eventually_emitted_once(failed), "job should signal it failed to unlock the accounts");
    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");

    QCOMPARE(openCounter, 1);
    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(passwordRequestsCancelled.count(), 1);
    QCOMPARE(failed.count(), 1);
    QCOMPARE(unlocked.count(), 0);

    QFile result(actualIni);
    QVERIFY2(result.exists(), "accounts file should still exist");
    QCOMPARE(test::slurp(actualIni), test::slurp(existingPasswordIniResource));
}

QTEST_MAIN(RequestAccountPasswordTest)

#include "request-account-password.moc"
