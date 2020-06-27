/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/account.h"

#include "../test-utils/output.h"
#include "../test-utils/spy.h"

#include "../../secrets/test-utils/random.h"

#include <QDateTime>
#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QVector>
#include <QtDebug>

static QString testIniResource(QLatin1String("test.ini"));
static QString testIniLockFile(QLatin1String("test.ini.lock"));

class HotpCounterUpdateTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void testCounterUpdate(void);
};

void HotpCounterUpdateTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::copyResourceAsWritable(":/counter-update/starting.ini", testIniResource), "test corpus INI resource should be available as file");
}

void HotpCounterUpdateTest::testCounterUpdate(void)
{
    const QString iniResource = test::path(testIniResource);
    const QString sampleAccountName(QLatin1String("valid-hotp-sample-1"));
    const QString originalToken(QLatin1String("755224"));
    const QString updatedToken(QLatin1String("287082"));

    const accounts::SettingsProvider settings([&iniResource](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(iniResource, QSettings::IniFormat);
        action(data);
    });

    QThread *thread = new QThread(this);
    QSignalSpy threadStarted(thread, &QThread::started);
    QSignalSpy threadFinished(thread, &QThread::finished);
    QSignalSpy threadCleaned(thread, &QThread::destroyed);

    thread->start();
    QVERIFY2(test::signal_eventually_emitted_once(threadStarted), "worker thread should be running by now");

    accounts::AccountSecret *secret = new accounts::AccountSecret(&test::fakeRandom);
    QSignalSpy existingPasswordNeeded(secret, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(secret, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(secret, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy keyAvailable(secret, &accounts::AccountSecret::keyAvailable);
    QSignalSpy passwordRequestsCancelled(secret, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy secretCleaned(secret, &accounts::AccountSecret::destroyed);

    accounts::AccountStorage *uut = new accounts::AccountStorage(settings, thread, secret);
    QSignalSpy error(uut, &accounts::AccountStorage::error);
    QSignalSpy loaded(uut, &accounts::AccountStorage::loaded);
    QSignalSpy accountAdded(uut, &accounts::AccountStorage::added);
    QSignalSpy accountRemoved(uut, &accounts::AccountStorage::removed);
    QSignalSpy storageDisposed(uut, &accounts::AccountStorage::disposed);
    QSignalSpy storageCleaned(uut, &accounts::AccountStorage::destroyed);

    // first phase: check that account objects can be loaded from storage

    // expect that unlocking is scheduled automatically, so advancing the event loop should trigger the signal
    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "(existing) password should be asked by now");
    QCOMPARE(newPasswordNeeded.count(), 0);

    QString password(QLatin1String("password"));
    secret->answerExistingPassword(password);

    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "(existing) password should have been accepted by now");
    QCOMPARE(password, QString(QLatin1String("********")));

    QVERIFY2(test::signal_eventually_emitted_once(keyAvailable, 2500), "key should have been derived by now");

    // expect that loading is scheduled automatically, so advancing the event loop should trigger the signal
    QVERIFY2(test::signal_eventually_emitted_once(loaded), "sample account should be loaded by now");
    QCOMPARE(uut->isLoaded(), true);
    QCOMPARE(uut->hasError(), false);
    QCOMPARE(error.count(), 0);
    QCOMPARE(accountAdded.count(), 1);
    QCOMPARE(accountAdded.at(0).at(0), sampleAccountName);

    accounts::Account *sampleAccount = uut->get(sampleAccountName);
    QVERIFY2(sampleAccount != nullptr, "get() should return the sample account");

    QSignalSpy sampleAccountRemoved(sampleAccount, &accounts::Account::removed);
    QSignalSpy sampleAccountCleaned(sampleAccount, &accounts::Account::destroyed);
    QSignalSpy sampleAccountTokenUpdated(sampleAccount, &accounts::Account::tokenChanged);

    QCOMPARE(sampleAccount->name(), sampleAccountName);
    QCOMPARE(sampleAccount->issuer(), QString());
    QCOMPARE(sampleAccount->algorithm(), accounts::Account::Hotp);
    QCOMPARE(sampleAccount->token(), QString());

    QCOMPARE(sampleAccount->counter(), 0ULL);
    QCOMPARE(sampleAccount->tokenLength(), 6);
    QCOMPARE(sampleAccount->offset(), std::nullopt);
    QCOMPARE(sampleAccount->checksum(), false);

    QFile initialLockFile(test::path(testIniLockFile));
    QVERIFY2(!initialLockFile.exists(), "initial: lock file should not be present anymore");

    QFile initialIni(iniResource);
    QVERIFY2(initialIni.exists(), "initial: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/counter-update/starting.ini")));

    // second phase: check that hotp tokens can be (re)computed
    sampleAccount->recompute();

    QVERIFY2(test::signal_eventually_emitted_once(sampleAccountTokenUpdated), "sample account token should be recomputed by now");
    QCOMPARE(sampleAccountTokenUpdated.at(0).at(0), originalToken);

    QFile afterComputingTokenLockFile(test::path(testIniLockFile));
    QVERIFY2(!afterComputingTokenLockFile.exists(), "after computing token: lock file should still not be present");

    QFile afterComputingTokenIni(iniResource);
    QVERIFY2(afterComputingTokenIni.exists(), "after computing token: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/counter-update/starting.ini")));

    // third phase: check that hotp counters can be updated in storage
    sampleAccount->advanceCounter();

    QVERIFY2(test::signal_eventually_emitted_twice(sampleAccountTokenUpdated), "sample account should be updated in storage by now");
    QCOMPARE(sampleAccountTokenUpdated.at(1).at(0), updatedToken);
    QCOMPARE(uut->hasError(), false);
    QCOMPARE(error.count(), 0);

    QFile afterUpdatingCounterLockFile(test::path(testIniLockFile));
    QVERIFY2(!afterUpdatingCounterLockFile.exists(), "after updating counter: lock file should not be present anymore");

    QFile afterUpdatingCounterIni(iniResource);
    QVERIFY2(afterUpdatingCounterIni.exists(), "after updating counter: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/counter-update/after-updating-counter.ini")));

    // fourth phase: check that disposing storage cleans up objects properly
    uut->dispose();

    QVERIFY2(test::signal_eventually_emitted_once(storageDisposed), "storage should be disposed of by now");
    QVERIFY2(test::signal_eventually_emitted_once(passwordRequestsCancelled), "account secret should have signalled cancellation by now");
    QVERIFY2(test::signal_eventually_emitted_once(sampleAccountCleaned), "sample account should be cleaned up by now");
    QVERIFY2(test::signal_eventually_emitted_once(secretCleaned), "account secret should be cleaned up by now");

    // fifth phase: check the sum-total effects

    QCOMPARE(error.count(), 0);
    QCOMPARE(loaded.count(), 1);
    QCOMPARE(accountAdded.count(), 1);
    QCOMPARE(accountRemoved.count(), 0);
    QCOMPARE(sampleAccountRemoved.count(), 0);
    QCOMPARE(sampleAccountCleaned.count(), 1);

    QFile finalIni(iniResource);
    QVERIFY2(finalIni.exists(), "final: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/counter-update/after-updating-counter.ini")));

    // sixth phase: wind down test
    QCOMPARE(threadFinished.count(), 0);

    thread->quit();
    QVERIFY2(test::signal_eventually_emitted_once(threadFinished), "thread should be finished by now");

    thread->deleteLater();
    QVERIFY2(test::signal_eventually_emitted_once(threadCleaned), "thread should be cleaned up by now");

    uut->deleteLater();
    QVERIFY2(test::signal_eventually_emitted_once(storageCleaned), "storage should be cleaned up by now");
}

QTEST_MAIN(HotpCounterUpdateTest)

#include "hotp-counter-update.moc"
