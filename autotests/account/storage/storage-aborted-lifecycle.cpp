/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/account.h"

#include "../../test-utils/spy.h"
#include "../test-utils/output.h"

#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QtDebug>

static QString testIniResource(QLatin1String("test.ini"));

class StorageAbortLifeCycleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void testLifecycle(void);
};

void StorageAbortLifeCycleTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::copyResourceAsWritable(QStringLiteral(":/storage-lifecycles/starting.ini"), testIniResource),
             "test corpus INI resource should be available as file");
}

void StorageAbortLifeCycleTest::testLifecycle(void)
{
    const QString iniResource = test::path(testIniResource);

    const accounts::SettingsProvider settings([&iniResource](const accounts::PersistenceAction &action) -> void {
        QSettings data(iniResource, QSettings::IniFormat);
        action(data);
    });

    accounts::AccountStorage *uut = accounts::AccountStorage::open(settings);
    QSignalSpy error(uut, &accounts::AccountStorage::error);
    QSignalSpy loaded(uut, &accounts::AccountStorage::loaded);
    QSignalSpy accountAdded(uut, &accounts::AccountStorage::added);
    QSignalSpy storageDisposed(uut, &accounts::AccountStorage::disposed);
    QSignalSpy storageCleaned(uut, &accounts::AccountStorage::destroyed);

    accounts::AccountSecret *secret = uut->secret();
    QSignalSpy existingPasswordNeeded(secret, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(secret, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(secret, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy keyAvailable(secret, &accounts::AccountSecret::keyAvailable);
    QSignalSpy passwordRequestsCancelled(secret, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy secretCleaned(secret, &accounts::AccountSecret::destroyed);

    // first phase:  expect that unlocking is scheduled automatically, so advancing the event loop should trigger the signal
    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "(existing) password should be asked by now");
    QCOMPARE(newPasswordNeeded.count(), 0);

    // second phase: check that disposing storage cleans up objects properly
    uut->dispose();

    QVERIFY2(test::signal_eventually_emitted_once(passwordRequestsCancelled), "account secret should have signalled cancellation by now");
    QVERIFY2(test::signal_eventually_emitted_once(storageDisposed), "storage should be disposed of by now");
    QVERIFY2(test::signal_eventually_emitted_once(secretCleaned), "account secret should be cleaned up by now");
    QVERIFY2(test::signal_eventually_emitted_once(storageCleaned), "storage should be cleaned up by now");

    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(accountAdded.count(), 0);
    QCOMPARE(loaded.count(), 0);
    QCOMPARE(error.count(), 1);
}

QTEST_MAIN(StorageAbortLifeCycleTest)

#include "storage-aborted-lifecycle.moc"
