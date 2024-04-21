/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/spy.h"

#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QtDebug>

class TestJob : public accounts::AccountJob // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
public:
    ~TestJob()
    {
        qDebug() << "TestJob is being destroyed on thread:" << QThread::currentThread() << "should be:" << thread();
    }

public Q_SLOTS:
    void run(void) override
    {
        Q_EMIT finished();
        qDebug() << "Did finish on thread:" << QThread::currentThread() << "should be:" << thread();
        qDebug() << "Current thread should NOT be finished, check:" << (QThread::currentThread()->isFinished() == false);
    }
};

class DispatchTest : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void liveDispatch(void);
    void coldDispatch(void);
};

/*
 * Test whether queueing jobs up-front before actually starting a worker thread works,
 * i.e. whether the job is automatically dispatched once the worker thread starts
 */
void DispatchTest::coldDispatch(void)
{
    QThread *thread = new QThread(this);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    QSignalSpy threadStarted(thread, &QThread::started);
    QSignalSpy threadFinished(thread, &QThread::finished);
    QSignalSpy threadCleaned(thread, &QThread::destroyed);

    accounts::Dispatcher *uut = new accounts::Dispatcher(thread);
    QSignalSpy dispatcherDispatched(uut, &accounts::Dispatcher::dispatch);
    QSignalSpy dispatcherCleaned(uut, &accounts::Dispatcher::destroyed);

    TestJob *job = new TestJob();
    QSignalSpy jobFinished(job, &TestJob::finished);
    QSignalSpy jobCleaned(job, &TestJob::destroyed);

    bool jobCallBackCalled = false;
    uut->queueAndProceed(job, [job, thread, &jobCallBackCalled](void) -> void {
        QCOMPARE(job->thread(), thread);
        jobCallBackCalled = true;
    });
    QVERIFY2(jobCallBackCalled, "signal setup call back should have been called for job");

    qDebug() << "About to start thread:" << thread;
    thread->start();
    QVERIFY2(test::signal_eventually_emitted_once(threadStarted), "worker thread should have started by now");

    QVERIFY2(test::signal_eventually_emitted_once(dispatcherDispatched), "dispatcher should have dispatched the job by now");
    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished by now");
    QVERIFY2(test::signal_eventually_emitted_once(jobCleaned), "job should be disposed of by now");

    QTimer::singleShot(0, thread, &QThread::quit);
    QVERIFY2(test::signal_eventually_emitted_once(threadFinished), "worker thread should be finished by now");
    QVERIFY2(test::signal_eventually_emitted_once(threadCleaned), "thread should be disposed of by now");

    uut->deleteLater();
    QVERIFY2(test::signal_eventually_emitted_once(dispatcherCleaned), "dispatcher should be disposed of by now");
}

/*
 * Test whether queueing jobs works while a worker thread is running already,
 * i.e. whether the job is dispatched straight away.
 */
void DispatchTest::liveDispatch(void)
{
    QThread *thread = new QThread(this);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    QSignalSpy threadStarted(thread, &QThread::started);
    QSignalSpy threadFinished(thread, &QThread::finished);
    QSignalSpy threadCleaned(thread, &QThread::destroyed);

    qDebug() << "About to start thread:" << thread;
    thread->start();
    QVERIFY2(test::signal_eventually_emitted_once(threadStarted), "worker thread should have started by now");

    accounts::Dispatcher *uut = new accounts::Dispatcher(thread);
    QSignalSpy dispatcherDispatched(uut, &accounts::Dispatcher::dispatch);
    QSignalSpy dispatcherCleaned(uut, &accounts::Dispatcher::destroyed);

    TestJob *job = new TestJob();
    QSignalSpy jobFinished(job, &TestJob::finished);
    QSignalSpy jobCleaned(job, &TestJob::destroyed);

    bool jobCallBackCalled = false;

    uut->queueAndProceed(job, [job, thread, &jobCallBackCalled](void) -> void {
        QCOMPARE(job->thread(), thread);
        jobCallBackCalled = true;
    });
    QVERIFY2(jobCallBackCalled, "signal setup call back should have been called for job");

    QVERIFY2(test::signal_eventually_emitted_once(dispatcherDispatched), "dispatcher should have dispatched the job by now");
    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished by now");
    QVERIFY2(test::signal_eventually_emitted_once(jobCleaned), "job should be disposed of by now");

    QTimer::singleShot(0, thread, &QThread::quit);
    QVERIFY2(test::signal_eventually_emitted_once(threadFinished), "worker thread should be finished by now");
    QVERIFY2(test::signal_eventually_emitted_once(threadCleaned), "thread should be disposed of by now");

    uut->deleteLater();
    QVERIFY2(test::signal_eventually_emitted_once(dispatcherCleaned), "dispatcher should be disposed of by now");
}

QTEST_MAIN(DispatchTest)

#include "dispatcher-dispatch.moc"
