/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/account.h"

#include "../test-utils/output.h"
#include "../../test-utils/spy.h"
#include "../../secrets/test-utils/random.h"

#include <QDateTime>
#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QVector>
#include <QtDebug>

#include <string.h>

static QString testIniResource(QLatin1String("test.ini"));
static QString testIniLockFile(QLatin1String("test.ini.lock"));

class StorageLifeCyclesTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void testLifecycle(void);
};

void StorageLifeCyclesTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::copyResourceAsWritable(QStringLiteral(":/storage-lifecycles/starting.ini"), testIniResource), "test corpus INI resource should be available as file");
}

void StorageLifeCyclesTest::testLifecycle(void)
{
    const QString iniResource = test::path(testIniResource);
    const QString initialAccountFullName(QLatin1String("autotests:valid-hotp-sample-1"));
    const QString addedAccountFullName(QLatin1String("autotests:valid-totp-sample-1"));
    const QString accountIssuer(QLatin1String("autotests"));
    const QString initialAccountName(QLatin1String("valid-hotp-sample-1"));
    const QString addedAccountName(QLatin1String("valid-totp-sample-1"));

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
    QCOMPARE(uut->isLoaded(), false);
    QCOMPARE(uut->hasError(), false);
    QCOMPARE(accountAdded.count(), 0);
    QCOMPARE(loaded.count(), 0);
    QCOMPARE(error.count(), 0);
    QVERIFY2(uut->isAccountStillAvailable(initialAccountName, accountIssuer),  "sample account, issuer should still be available");
    QVERIFY2(uut->isAccountStillAvailable(initialAccountFullName),  "sample account (full name) should still be available");
    QVERIFY2(uut->isAccountStillAvailable(addedAccountName, accountIssuer),  "new account, issuer should still be available");
    QVERIFY2(uut->isAccountStillAvailable(addedAccountFullName),  "new account (full name) should still be available");
    QCOMPARE(uut->accounts(), QVector<QString>());

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
    QCOMPARE(accountAdded.at(0).at(0), initialAccountFullName);

    QVERIFY2(!uut->isAccountStillAvailable(initialAccountName, accountIssuer),  "sample account, issuer should no longer be available");
    QVERIFY2(!uut->isAccountStillAvailable(initialAccountFullName),  "sample account (full name) should no longer be available");
    QVERIFY2(uut->isAccountStillAvailable(addedAccountName, accountIssuer),  "new account, issuer should still be available");
    QVERIFY2(uut->isAccountStillAvailable(addedAccountFullName),  "new account (full name) should still be available");
    QCOMPARE(uut->accounts(), QVector<QString>() << initialAccountFullName);

    QVERIFY2(uut->contains(initialAccountName, accountIssuer), "contains(name, issuer) should report the sample account");
    QVERIFY2(uut->contains(initialAccountFullName), "contains(full name) should report the sample account");

    accounts::Account *initialAccount = uut->get(initialAccountFullName);
    QVERIFY2(initialAccount != nullptr, "get(full name) should return the sample account");
    QCOMPARE(uut->get(initialAccountName, accountIssuer), initialAccount);

    QSignalSpy initialAccountRemoved(initialAccount, &accounts::Account::removed);
    QSignalSpy initialAccountCleaned(initialAccount, &accounts::Account::destroyed);

    QCOMPARE(initialAccount->name(), initialAccountName);
    QCOMPARE(initialAccount->issuer(), accountIssuer);
    QCOMPARE(initialAccount->algorithm(), accounts::Account::Hotp);
    QCOMPARE(initialAccount->token(), QString());

    QCOMPARE(initialAccount->counter(), 42ULL);
    QCOMPARE(initialAccount->tokenLength(), 7);
    QCOMPARE(initialAccount->offset(), std::nullopt);
    QCOMPARE(initialAccount->checksum(), false);

    QFile initialLockFile(test::path(testIniLockFile));
    QVERIFY2(!initialLockFile.exists(), "initial: lock file should not be present anymore");

    QFile initialIni(iniResource);
    QVERIFY2(initialIni.exists(), "initial: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/storage-lifecycles/starting.ini")));

    // second phase: check that account objects can be removed from storage
    QCOMPARE(accountRemoved.count(), 0);

    initialAccount->remove();

    QVERIFY2(test::signal_eventually_emitted_once(accountRemoved), "sample account should be removed from storage by now");
    QCOMPARE(accountRemoved.at(0).at(0), initialAccountFullName);

    QVERIFY2(uut->isAccountStillAvailable(initialAccountName, accountIssuer),  "sample account, issuer should again be available");
    QVERIFY2(uut->isAccountStillAvailable(initialAccountFullName),  "sample account (full name) should again be available");
    QVERIFY2(uut->isAccountStillAvailable(addedAccountName, accountIssuer),  "new account, issuer should still be available");
    QVERIFY2(uut->isAccountStillAvailable(addedAccountFullName),  "new account (full name) should still be available");
    QCOMPARE(uut->accounts(), QVector<QString>());

    QVERIFY2(!uut->contains(initialAccountName, accountIssuer), "contains(name, issuer) should no longer report the sample account");
    QVERIFY2(!uut->contains(initialAccountFullName), "contains(full name) should no longer report the sample account");
    QVERIFY2(uut->get(initialAccountFullName) == nullptr, "get(full name) should no longer return the sample account");
    QVERIFY2(uut->get(initialAccountName, accountIssuer) == nullptr, "get(name, issuer) should no longer return the sample account");

    QFile afterRemovingLockFile(test::path(testIniLockFile));
    QVERIFY2(!afterRemovingLockFile.exists(), "after removing: lock file should not be present anymore");

    QFile afterRemovingIni(iniResource);
    QVERIFY2(afterRemovingIni.exists(), "after removing: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/storage-lifecycles/after-removing.ini")));

    QVERIFY2(test::signal_eventually_emitted_once(initialAccountRemoved), "sample account should have signalled its own removal by now");
    QVERIFY2(test::signal_eventually_emitted_once(initialAccountCleaned), "sample account should be cleaned up by now");

    // third phase: check that new account objects can be added to storage
    uut->addTotp(addedAccountName, accountIssuer, QLatin1String("NBSWY3DPFQQHO33SNRSCC==="), 8U, 42U);

    QVERIFY2(test::signal_eventually_emitted_twice(accountAdded), "new account should be added to storage by now");
    QCOMPARE(error.count(), 0);
    QCOMPARE(accountAdded.at(1).at(0), addedAccountFullName);

    QVERIFY2(uut->isAccountStillAvailable(initialAccountName, accountIssuer),  "sample account, issuer should again still be available");
    QVERIFY2(uut->isAccountStillAvailable(initialAccountFullName),  "sample account (full name) should again still be available");
    QVERIFY2(!uut->isAccountStillAvailable(addedAccountName, accountIssuer),  "new account, issuer should no longer be available");
    QVERIFY2(!uut->isAccountStillAvailable(addedAccountFullName),  "new account (full name) should no longer be available");
    QCOMPARE(uut->accounts(), QVector<QString>() << addedAccountFullName);

    QVERIFY2(uut->contains(addedAccountName, accountIssuer), "contains(name, issuer) should report the new account");
    QVERIFY2(uut->contains(addedAccountFullName), "contains(full name) should report the new account");

    accounts::Account *addedAccount = uut->get(addedAccountFullName);
    QVERIFY2(addedAccount != nullptr, "get(full name) should return the new account");
    QCOMPARE(uut->get(addedAccountName, accountIssuer), addedAccount);

    QSignalSpy addedAccountRemoved(addedAccount, &accounts::Account::removed);
    QSignalSpy addedAccountCleaned(addedAccount, &accounts::Account::destroyed);

    QCOMPARE(addedAccount->name(), addedAccountName);
    QCOMPARE(addedAccount->issuer(), accountIssuer);
    QCOMPARE(addedAccount->algorithm(), accounts::Account::Totp);
    QCOMPARE(addedAccount->token(), QString());

    QCOMPARE(addedAccount->timeStep(), 42U);
    QCOMPARE(addedAccount->tokenLength(), 8);
    QCOMPARE(addedAccount->epoch(), QDateTime::fromMSecsSinceEpoch(0));
    QCOMPARE(addedAccount->hash(), accounts::Account::Sha1);

    QFile afterAddingLockFile(test::path(testIniLockFile));
    QVERIFY2(!afterAddingLockFile.exists(), "after adding: lock file should not be present anymore");

    QFile afterAddingIni(iniResource);
    QVERIFY2(afterAddingIni.exists(), "after adding: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/storage-lifecycles/after-adding.ini")));

    // fourth phase: check that disposing storage cleans up objects properly
    uut->dispose();

    QVERIFY2(!uut->isAccountStillAvailable(initialAccountName, accountIssuer),  "sample account, issuer should again no longer be available");
    QVERIFY2(!uut->isAccountStillAvailable(initialAccountFullName),  "sample account (full name) should again no longer be available");
    QVERIFY2(!uut->isAccountStillAvailable(addedAccountName, accountIssuer),  "new account, issuer should no longer be available still");
    QVERIFY2(!uut->isAccountStillAvailable(addedAccountFullName),  "new account (full name) should no longer be available still");
    QCOMPARE(uut->accounts(), QVector<QString>());

    QVERIFY2(!uut->contains(addedAccountName, accountIssuer), "contains(name, issuer) should no longer report the new account");
    QVERIFY2(!uut->contains(addedAccountFullName), "contains(full name) should no longer report the new account");
    QVERIFY2(uut->get(addedAccountFullName) == nullptr, "get(full name) should no longer return the new account");
    QVERIFY2(uut->get(addedAccountName, accountIssuer) == nullptr, "get(name, issuer) should no longer return the new account");

    QVERIFY2(test::signal_eventually_emitted_once(passwordRequestsCancelled), "account secret should have signalled cancellation by now");

    /*
     * The disposed() signal is the hook for consuming code to know when to drop objects.
     * Check that it is emitted *before* account objects are actually destroyed, i.e that the signal arrives before, and not after the fact.
     */
    QVERIFY2(test::signal_eventually_emitted_once(storageDisposed), "storage should be disposed of by now");
    QCOMPARE(addedAccountCleaned.count(), 0);

    QVERIFY2(test::signal_eventually_emitted_once(addedAccountCleaned), "new account should be disposed of by now");
    QVERIFY2(test::signal_eventually_emitted_once(secretCleaned), "account secret should be cleaned up by now");

    // fifth phase: check the sum-total effects

    QCOMPARE(error.count(), 0);
    QCOMPARE(loaded.count(), 1);
    QCOMPARE(addedAccountRemoved.count(), 0);
    QCOMPARE(accountAdded.count(), 2);
    QCOMPARE(accountRemoved.count(), 1);
    QCOMPARE(initialAccountRemoved.count(), 1);
    QCOMPARE(initialAccountCleaned.count(), 1);
    QCOMPARE(addedAccountRemoved.count(), 0);
    QCOMPARE(addedAccountCleaned.count(), 1);

    QFile finalIni(iniResource);
    QVERIFY2(finalIni.exists(), "final: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/storage-lifecycles/after-adding.ini")));

    // sixth phase: wind down test
    QCOMPARE(threadFinished.count(), 0);

    thread->quit();
    QVERIFY2(test::signal_eventually_emitted_once(threadFinished), "thread should be finished by now");

    thread->deleteLater();
    QVERIFY2(test::signal_eventually_emitted_once(threadCleaned), "thread should be cleaned up by now");

    uut->deleteLater();
    QVERIFY2(test::signal_eventually_emitted_once(storageCleaned), "storage should be cleaned up by now");
}

QTEST_MAIN(StorageLifeCyclesTest)

#include "storage-object-lifecycles.moc"
