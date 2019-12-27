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

class StorageDefaultLifeCycleTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void testLifecycle(void);
};

void StorageDefaultLifeCycleTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::copyResourceAsWritable(":/storage-lifecycles/starting.ini", testIniResource), "test corpus INI resource should be available as file");
}

void StorageDefaultLifeCycleTest::testLifecycle(void)
{
    const QString iniResource = test::path(testIniResource);
    const QString sampleAccountName(QLatin1String("valid-hotp-sample-1"));

    const accounts::SettingsProvider settings([&iniResource](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(iniResource, QSettings::IniFormat);
        action(data);
    });

    accounts::AccountStorage *uut = accounts::AccountStorage::open(settings);
    QSignalSpy accountAdded(uut, &accounts::AccountStorage::added);
    QSignalSpy storageDisposed(uut, &accounts::AccountStorage::disposed);
    QSignalSpy storageCleaned(uut, &accounts::AccountStorage::destroyed);

    // first phase: check that account objects can be loaded from storage

    // expect that loading is scheduled automatically, so advancing the event loop should trigger the signal
    QVERIFY2(test::signal_eventually_emitted_once(accountAdded), "sample account should be loaded by now");
    QCOMPARE(accountAdded.at(0).at(0), sampleAccountName);

    accounts::Account *sampleAccount = uut->get(sampleAccountName);
    QVERIFY2(sampleAccount != nullptr, "get() should return the sample account");

    QSignalSpy sampleAccountCleaned(sampleAccount, &accounts::Account::destroyed);

    // second phase: check that disposing storage cleans up objects properly
    uut->dispose();

    QVERIFY2(test::signal_eventually_emitted_once(storageDisposed), "storage should be disposed of by now");
    QVERIFY2(test::signal_eventually_emitted_once(sampleAccountCleaned), "sample account should be cleaned up by now");
    QVERIFY2(test::signal_eventually_emitted_once(storageCleaned), "storage should be cleaned up by now");
}

QTEST_MAIN(StorageDefaultLifeCycleTest)

#include "storage-default-lifecycle.moc"
