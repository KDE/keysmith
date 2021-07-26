/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath/oath.h"

#include <QTest>
#include <QtDebug>

#include <limits>

class TimeStepCountConversionTest: public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void invalidCount(void);
    void invalidCount_data(void);
    void validCount(void);
    void validCount_data(void);
};


static void define_valid_test_case(const QDateTime &epoch, uint timeStep, quint64 count, const QDateTime &expected)
{
    static const QString testCase(QStringLiteral("%1 + %2 * 1000 * %3 ... %4"));
    QTest::newRow(qPrintable(testCase.arg(epoch.toMSecsSinceEpoch()).arg(timeStep).arg(count).arg(expected.toMSecsSinceEpoch()))) << epoch << timeStep << count << expected;
}

static void define_invalid_test_case(const QDateTime &epoch, uint timeStep, quint64 count)
{
    static const QString testCase(QStringLiteral("%1 + %2 * 1000 * %3"));
    QTest::newRow(qPrintable(testCase.arg(epoch.toMSecsSinceEpoch()).arg(timeStep).arg(count))) << epoch << timeStep << count;
}

void TimeStepCountConversionTest::validCount(void)
{
    QFETCH(QDateTime, epoch);
    QFETCH(uint, timeStep);
    QFETCH(quint64, count);

    auto result = oath::fromCounter(count, epoch, timeStep);
    QVERIFY2(result, "should be able to determine the datetime associated with the counter");
    QTEST(*result, "expected");
}

void TimeStepCountConversionTest::validCount_data(void)
{
    QTest::addColumn<QDateTime>("epoch");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<quint64>("count");
    QTest::addColumn<QDateTime>("expected");

    const qint64 min = std::numeric_limits<qint64>::min();
    const qint64 s2038 = std::numeric_limits<qint32>::max();

    // few basic cases
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 30UL, 0ULL, QDateTime::fromMSecsSinceEpoch(0LL));
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 30UL, 1ULL, QDateTime::fromMSecsSinceEpoch(30'000LL));
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(-30'000LL), 30UL, 1ULL, QDateTime::fromMSecsSinceEpoch(0LL));

    // checking max 'classic' 32bit unix time for counting utc seconds
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 30UL, s2038/ 30LL, QDateTime::fromMSecsSinceEpoch(30'000LL * (s2038 / 30LL)));
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1UL, s2038, QDateTime::fromMSecsSinceEpoch(1'000LL * s2038));

    // finding the actual upper limit(s) which QDateTime can cope with
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1ULL, 0x7fffffffff921fd8ULL / 1000ULL, QDateTime::fromMSecsSinceEpoch(0x7fffffffff921fd8LL));
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(808LL), 1ULL, 0x7fffffffff921fd8ULL / 1000ULL - 1ULL, QDateTime::fromMSecsSinceEpoch(0x7fffffffff921fd8LL - 192LL));
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(807LL), 1ULL, 0x7fffffffff921fd8ULL / 1000ULL, QDateTime::fromMSecsSinceEpoch(0x7fffffffff921fd8LL + 807LL));

    // finding the lower limit(s) which QDateTime can cope with
    define_valid_test_case(QDateTime::fromMSecsSinceEpoch(min), 1UL, 0ULL, QDateTime::fromMSecsSinceEpoch(min));
}

void TimeStepCountConversionTest::invalidCount(void)
{
    QFETCH(QDateTime, epoch);
    QFETCH(uint, timeStep);
    QFETCH(quint64, count);

    QVERIFY2(!oath::fromCounter(count, epoch, timeStep), "should not be able to associate a datetime with the counter");
}

void TimeStepCountConversionTest::invalidCount_data(void)
{
    QTest::addColumn<QDateTime>("epoch");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<quint64>("count");

    const quint64 max = std::numeric_limits<quint64>::max();

    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1UL, max / 2);
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1UL, max);
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(std::numeric_limits<qint64>::min()), 1UL, max);
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(std::numeric_limits<qint64>::min()), 1UL, max / 1000ULL + 1ULL);

    // fits within qint64 ultimately but QDateTime gets confused about it
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1ULL, (quint64) (max / 1000LL));
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1ULL, (quint64) (max >> 9));
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(0LL), 1ULL, (quint64) (max >> 10) + (max >> 11L));
    define_invalid_test_case(QDateTime::fromMSecsSinceEpoch(808LL), 1ULL, 0x7fffffffffffffffULL / 1000ULL);
}

QTEST_APPLESS_MAIN(TimeStepCountConversionTest)

#include "convert-timestep-counter.moc"
