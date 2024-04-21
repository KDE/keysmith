/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/job.h"
#include "../test-utils/spy.h"

#include <QSignalSpy>
#include <QTest>
#include <QtDebug>

class NextTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void coldNext(void);
};

void NextTest::coldNext(void)
{
    test::JobSignals *jobSignals = new test::JobSignals(this);

    QThread *thread = new QThread(this);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(jobSignals, &test::JobSignals::quit, thread, &QThread::quit);

    QSignalSpy threadStarted(thread, &QThread::started);
    QSignalSpy threadFinished(thread, &QThread::finished);
    QSignalSpy threadCleaned(thread, &QThread::destroyed);

    accounts::Dispatcher *uut = new accounts::Dispatcher(thread);
    QSignalSpy dispatcherDispatched(uut, &accounts::Dispatcher::dispatch);

    test::TestJob *firstJob = new test::TestJob();
    QSignalSpy firstJobRunning(firstJob, &test::TestJob::running);
    QSignalSpy firstJobFinished(firstJob, &test::TestJob::finished);
    QSignalSpy firstJobCleaned(firstJob, &test::TestJob::destroyed);

    uut->queueAndProceed(firstJob, [firstJob, thread, jobSignals](void) -> void {
        QCOMPARE(firstJob->thread(), thread);
        QObject::connect(jobSignals, &test::JobSignals::first, firstJob, &test::TestJob::finish);
    });

    test::TestJob *secondJob = new test::TestJob();
    QSignalSpy secondJobRunning(secondJob, &test::TestJob::running);
    QSignalSpy secondJobFinished(secondJob, &test::TestJob::finished);
    QSignalSpy secondJobCleaned(secondJob, &test::TestJob::destroyed);

    uut->queueAndProceed(secondJob, [secondJob, thread, jobSignals](void) -> void {
        QCOMPARE(secondJob->thread(), thread);
        QObject::connect(jobSignals, &test::JobSignals::second, secondJob, &test::TestJob::finish);
    });

    thread->start();

    QVERIFY2(test::signal_eventually_emitted_once(threadStarted), "worker thread should have started by now");

    QVERIFY2(test::signal_eventually_emitted_once(dispatcherDispatched), "dispatcher should have dispatched the first job by now");

    QVERIFY2(test::signal_eventually_emitted_once(firstJobRunning), "first job should be running by now");
    QCOMPARE(secondJobRunning.count(), 0); // and second job should not yet be

    Q_EMIT jobSignals->first();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 500);

    QVERIFY2(test::signal_eventually_emitted_once(firstJobFinished), "first job should be finished by now");
    QVERIFY2(test::signal_eventually_emitted_once(firstJobCleaned), "first job should be disposed of by now");

    QVERIFY2(test::signal_eventually_emitted_twice(dispatcherDispatched), "dispatcher should have dispatched the second job by now");
    QVERIFY2(test::signal_eventually_emitted_once(secondJobRunning), "second job should be dispatched and running by now");

    Q_EMIT jobSignals->second();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 500);

    QVERIFY2(test::signal_eventually_emitted_once(secondJobFinished), "second job should be finished by now");
    QVERIFY2(test::signal_eventually_emitted_once(secondJobCleaned), "second job should be disposed of by now");

    Q_EMIT jobSignals->quit();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 500);

    QVERIFY2(test::signal_eventually_emitted_once(threadFinished), "worker thread should be finished by now");
    QVERIFY2(test::signal_eventually_emitted_once(threadCleaned), "thread should be disposed of by now");

    uut->deleteLater();
    jobSignals->deleteLater();
}

QTEST_MAIN(NextTest)

#include "dispatcher-next.moc"
