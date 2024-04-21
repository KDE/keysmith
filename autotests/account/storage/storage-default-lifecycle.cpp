/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/account.h"

#include "../../test-utils/spy.h"
#include "../test-utils/output.h"

#include <QDateTime>
#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QVector>
#include <QtDebug>

static QString testIniResource(QLatin1String("test.ini"));

class StorageDefaultLifeCycleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void testLifecycle(void);
};

void StorageDefaultLifeCycleTest::initTestCase(void)
{
    QVERIFY2(test::ensureOutputDirectory(), "output directory should be available");
    QVERIFY2(test::copyResourceAsWritable(QStringLiteral(":/storage-lifecycles/starting.ini"), testIniResource),
             "test corpus INI resource should be available as file");
}

void StorageDefaultLifeCycleTest::testLifecycle(void)
{
    const QString iniResource = test::path(testIniResource);
    const QString samleAccountFullName(QLatin1String("autotests:valid-hotp-sample-1"));

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

    // first phase: check that account objects can be loaded from storage
    QCOMPARE(uut->isLoaded(), false);
    QCOMPARE(uut->hasError(), false);

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
    QCOMPARE(accountAdded.at(0).at(0), samleAccountFullName);

    accounts::Account *sampleAccount = uut->get(samleAccountFullName);
    QVERIFY2(sampleAccount != nullptr, "get() should return the sample account");

    QSignalSpy sampleAccountCleaned(sampleAccount, &accounts::Account::destroyed);

    // second phase: check that disposing storage cleans up objects properly
    uut->dispose();

    QVERIFY2(test::signal_eventually_emitted_once(passwordRequestsCancelled), "account secret should have signalled cancellation by now");
    QVERIFY2(test::signal_eventually_emitted_once(storageDisposed), "storage should be disposed of by now");
    QVERIFY2(test::signal_eventually_emitted_once(sampleAccountCleaned), "sample account should be cleaned up by now");
    QVERIFY2(test::signal_eventually_emitted_once(secretCleaned), "account secret should be cleaned up by now");
    QVERIFY2(test::signal_eventually_emitted_once(storageCleaned), "storage should be cleaned up by now");
}

QTEST_MAIN(StorageDefaultLifeCycleTest)

#include "storage-default-lifecycle.moc"
