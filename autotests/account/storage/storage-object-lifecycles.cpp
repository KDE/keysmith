/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/account.h"

#include "../test-utils/output.h"
#include "../test-utils/spy.h"

#include <QDateTime>
#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QVector>
#include <QtDebug>

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
    QVERIFY2(test::copyResourceAsWritable(":/storage-lifecycles/starting.ini", testIniResource), "test corpus INI resource should be available as file");
}

void StorageLifeCyclesTest::testLifecycle(void)
{
    const QString iniResource = test::path(testIniResource);
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

    accounts::AccountStorage *uut = new accounts::AccountStorage(settings, thread);
    QSignalSpy accountAdded(uut, &accounts::AccountStorage::added);
    QSignalSpy accountRemoved(uut, &accounts::AccountStorage::removed);
    QSignalSpy storageDisposed(uut, &accounts::AccountStorage::disposed);
    QSignalSpy storageCleaned(uut, &accounts::AccountStorage::destroyed);

    // first phase: check that account objects can be loaded from storage
    QCOMPARE(accountAdded.count(), 0);
    QVERIFY2(uut->isNameStillAvailable(initialAccountName),  "sample account name should still be available");
    QVERIFY2(uut->isNameStillAvailable(addedAccountName),  "new account name should still be available");
    QCOMPARE(uut->accounts(), QVector<QString>());

    // expect that loading is scheduled automatically, so advancing the event loop should trigger the signal
    QVERIFY2(test::signal_eventually_emitted_once(accountAdded), "sample account should be loaded by now");
    QCOMPARE(accountAdded.at(0).at(0), initialAccountName);

    QVERIFY2(!uut->isNameStillAvailable(initialAccountName),  "sample account name should no longer be available");
    QVERIFY2(uut->isNameStillAvailable(addedAccountName),  "new account name should still be available");
    QCOMPARE(uut->accounts(), QVector<QString>() << initialAccountName);

    QVERIFY2(uut->contains(initialAccountName), "contains() should report the sample account");

    accounts::Account *initialAccount = uut->get(initialAccountName);
    QVERIFY2(initialAccount != nullptr, "get() should return the sample account");

    QSignalSpy initialAccountRemoved(initialAccount, &accounts::Account::removed);
    QSignalSpy initialAccountCleaned(initialAccount, &accounts::Account::destroyed);

    QCOMPARE(initialAccount->name(), initialAccountName);
    QCOMPARE(initialAccount->algorithm(), accounts::Account::Hotp);
    QCOMPARE(initialAccount->token(), QString());

    QCOMPARE(initialAccount->counter(), 42ULL);
    QCOMPARE(initialAccount->tokenLength(), 7);
    QCOMPARE(initialAccount->offset(), -1);
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
    QCOMPARE(accountRemoved.at(0).at(0), initialAccountName);

    QVERIFY2(uut->isNameStillAvailable(initialAccountName),  "sample account name should again be available");
    QVERIFY2(uut->isNameStillAvailable(addedAccountName),  "new account name should still be available");
    QCOMPARE(uut->accounts(), QVector<QString>());

    QVERIFY2(!uut->contains(initialAccountName), "contains() should no longer report the sample account");
    QVERIFY2(uut->get(initialAccountName) == nullptr, "get() should no longer return the sample account");

    QFile afterRemovingLockFile(test::path(testIniLockFile));
    QVERIFY2(!afterRemovingLockFile.exists(), "after removing: lock file should not be present anymore");

    QFile afterRemovingIni(iniResource);
    QVERIFY2(afterRemovingIni.exists(), "after removing: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/storage-lifecycles/after-removing.ini")));

    QVERIFY2(test::signal_eventually_emitted_once(initialAccountRemoved), "sample account should have signalled its own removal by now");
    QVERIFY2(test::signal_eventually_emitted_once(initialAccountCleaned), "sample account should be cleaned up by now");

    // third phase: check that new account objects can be added to storage
    uut->addTotp(addedAccountName, QLatin1String("NBSWY3DPFQQHO33SNRSCCCQ="), 42, 8);

    QVERIFY2(test::signal_eventually_emitted_twice(accountAdded), "new account should be added to storage by now");
    QCOMPARE(accountAdded.at(1).at(0), addedAccountName);

    QVERIFY2(uut->isNameStillAvailable(initialAccountName),  "sample account name should again still be available");
    QVERIFY2(!uut->isNameStillAvailable(addedAccountName),  "new account name should no longer be available");
    QCOMPARE(uut->accounts(), QVector<QString>() << addedAccountName);

    QVERIFY2(uut->contains(addedAccountName), "contains() should report the new account");

    accounts::Account *addedAccount = uut->get(addedAccountName);
    QVERIFY2(addedAccount != nullptr, "get() should return the new account");

    QSignalSpy addedAccountRemoved(addedAccount, &accounts::Account::removed);
    QSignalSpy addedAccountCleaned(addedAccount, &accounts::Account::destroyed);

    QCOMPARE(addedAccount->name(), addedAccountName);
    QCOMPARE(addedAccount->algorithm(), accounts::Account::Totp);
    QCOMPARE(addedAccount->token(), QString());

    QCOMPARE(addedAccount->timeStep(), 42U);
    QCOMPARE(addedAccount->tokenLength(), 8);
    QCOMPARE(addedAccount->epoch(), QDateTime::fromMSecsSinceEpoch(0));
    QCOMPARE(addedAccount->hash(), accounts::Account::Default);

    QFile afterAddingLockFile(test::path(testIniLockFile));
    QVERIFY2(!afterAddingLockFile.exists(), "after adding: lock file should not be present anymore");

    QFile afterAddingIni(iniResource);
    QVERIFY2(afterAddingIni.exists(), "after adding: accounts file should still exist");
    QCOMPARE(test::slurp(iniResource), test::slurp(QLatin1String(":/storage-lifecycles/after-adding.ini")));

    // fourth phase: check that disposing storage cleans up objects properly
    uut->dispose();

    QVERIFY2(!uut->isNameStillAvailable(initialAccountName),  "sample account name should again no longer be available");
    QVERIFY2(!uut->isNameStillAvailable(addedAccountName),  "new account name should no longer be available still");
    QCOMPARE(uut->accounts(), QVector<QString>());

    QVERIFY2(!uut->contains(addedAccountName), "contains() should no longer report the new account");
    QVERIFY2(uut->get(addedAccountName) == nullptr, "get() should no longer return the new account");

    /*
     * The disposed() signal is the hook for consuming code to know when to drop objects.
     * Check that it is emitted *before* account objects are actually destroyed, i.e that the signal arrives before, and not after the fact.
     */
    QVERIFY2(test::signal_eventually_emitted_once(storageDisposed), "storage should be disposed of by now");
    QCOMPARE(addedAccountCleaned.count(), 0);

    QVERIFY2(test::signal_eventually_emitted_once(addedAccountCleaned), "new account should be disposed of by now");

    // fifth phase: check the sum-total effects

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
