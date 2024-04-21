/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "model/accounts.h"

#include <QDateTime>
#include <QObject>
#include <QTest>

#include <functional>

class MillisecondsLeftForTokenTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMillisecondsLeftForToken(void);
    void testMillisecondsLeftForToken_data(void);
};

static void define_test_case(const char *testCase, const QDateTime &epoch, uint timeStep, qint64 now, qint64 left)
{
    QTest::newRow(qPrintable(QLatin1String(testCase))) << epoch << timeStep << now << left;
}

void MillisecondsLeftForTokenTest::testMillisecondsLeftForToken(void)
{
    QFETCH(QDateTime, epoch);
    QFETCH(uint, timeStep);
    QFETCH(qint64, now);

    const std::function<qint64(void)> clock([now](void) -> qint64 {
        return now;
    });

    QTEST(model::millisecondsLeftForToken(epoch, timeStep, clock), "left");
}

void MillisecondsLeftForTokenTest::testMillisecondsLeftForToken_data(void)
{
    QTest::addColumn<QDateTime>("epoch");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<qint64>("now");
    QTest::addColumn<qint64>("left");

    define_test_case("at the epoch itself", QDateTime::fromMSecsSinceEpoch(0), 30U, 0LL, 30LL * 1000LL);

    const QString withLeftToGo(QLatin1String("with %1ms left to go"));
    for (qint64 left = 1000LL; left > 0; --left) {
        QTest::newRow(qPrintable(withLeftToGo.arg(left))) << QDateTime::fromMSecsSinceEpoch(0) << 1U << 1000LL - left << left;
    }

    define_test_case("at the start of 'next' token", QDateTime::fromMSecsSinceEpoch(0), 30U, 30LL * 1000LL, 30LL * 1000LL);
    define_test_case("using a non-standard epoch (in the past)", QDateTime::fromMSecsSinceEpoch(15LL * 1000LL), 30U, 30LL * 1000LL, 15LL * 1000LL);

    define_test_case("using a non-standard epoch (in the future)", QDateTime::fromMSecsSinceEpoch(25LL * 1000LL), 30U, 0LL, 25LL * 1000LL);
}

QTEST_APPLESS_MAIN(MillisecondsLeftForTokenTest)

#include "milliseconds-left-for-token.moc"
