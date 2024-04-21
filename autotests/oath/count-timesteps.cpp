/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath/oath.h"

#include <QTest>
#include <QtDebug>

class TimeStepCounterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void invalidCount(void);
    void invalidCount_data(void);
    void validCount(void);
    void validCount_data(void);
    void rfcTestVector(void);
    void rfcTestVector_data(void);
};

static void define_valid_test_case(const QDateTime &epoch, qint64 now, uint timeStep, quint64 expected)
{
    static const QString testCase(QLatin1String("%2 - %1 (%3) ... %4"));
    QTest::newRow(qPrintable(testCase.arg(epoch.toMSecsSinceEpoch()).arg(now).arg(timeStep).arg(expected))) << epoch << now << timeStep << expected;
}

static void define_invalid_test_case(const QDateTime &epoch, qint64 now, uint timeStep)
{
    static const QString testCase(QLatin1String("%2 - %1 (%3)"));
    QTest::newRow(qPrintable(testCase.arg(epoch.toMSecsSinceEpoch()).arg(now).arg(timeStep))) << epoch << now << timeStep;
}

void TimeStepCounterTest::validCount(void)
{
    QFETCH(QDateTime, epoch);
    QFETCH(qint64, now);
    QFETCH(uint, timeStep);
    QFETCH(quint64, expected);
    const std::function<qint64(void)> clock([now](void) -> qint64 {
        return now;
    });
    QCOMPARE(oath::count(epoch, timeStep, clock), std::optional<quint64>(expected));
}

void TimeStepCounterTest::validCount_data(void)
{
    QTest::addColumn<QDateTime>("epoch");
    QTest::addColumn<qint64>("now");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<quint64>("expected");

    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1500LL, 30UL, 0ULL);
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1500LL, 1UL, 1ULL);
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 30500LL, 30UL, 1ULL);
}

void TimeStepCounterTest::invalidCount(void)
{
    QFETCH(QDateTime, epoch);
    QFETCH(qint64, now);
    QFETCH(uint, timeStep);
    const std::function<qint64(void)> clock([now](void) -> qint64 {
        return now;
    });
    QVERIFY2(!oath::count(epoch, timeStep, clock), "counting timesteps should fail (invalid inputs)");
}

void TimeStepCounterTest::invalidCount_data(void)
{
    QTest::addColumn<QDateTime>("epoch");
    QTest::addColumn<qint64>("now");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<quint64>("expected");

    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(1500LL), 0LL, 30UL);
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1500LL, 0UL);
}

static void define_rfc_test_case(const QString &datetime, quint64 expected)
{
    static QString testCase(QLatin1String("%1 ... 0x%2"));
    const QDateTime dt = QDateTime::fromString(datetime, Qt::ISODate);
    QTest::newRow(qPrintable(testCase.arg(datetime).arg(expected, 8, 16, QLatin1Char('0')))) << dt << expected;
}

/*
 * (Static) test vector params obtained from RFC-6238
 * https://tools.ietf.org/html/rfc6238#page-9
 */

void TimeStepCounterTest::rfcTestVector(void)
{
    QFETCH(QDateTime, now);
    const QDateTime epoch = QDateTime::fromMSecsSinceEpoch(0);
    const uint timeStep = 30U;

    std::optional<quint64> counter = oath::count(epoch, timeStep, [now](void) -> qint64 {
        return now.toMSecsSinceEpoch();
    });

    QVERIFY2(counter, "should be able to count timesteps using the settings of the RFC test vector");
    QTEST(*counter, "rfc-test-vector");
}

void TimeStepCounterTest::rfcTestVector_data(void)
{
    QTest::addColumn<QDateTime>("now");
    QTest::addColumn<quint64>("rfc-test-vector");

    define_rfc_test_case(QStringLiteral("1970-01-01 00:00:59Z"), 0x1ULL);
    define_rfc_test_case(QStringLiteral("2005-03-18 01:58:29Z"), 0x23523ECULL);
    define_rfc_test_case(QStringLiteral("2005-03-18 01:58:31Z"), 0x23523EDULL);
    define_rfc_test_case(QStringLiteral("2009-02-13 23:31:30Z"), 0x273EF07ULL);
    define_rfc_test_case(QStringLiteral("2033-05-18 03:33:20Z"), 0x3F940AAULL);
    define_rfc_test_case(QStringLiteral("2603-10-11 11:33:20Z"), 0x27BC86AAULL);
}

QTEST_APPLESS_MAIN(TimeStepCounterTest)

#include "count-timesteps.moc"
