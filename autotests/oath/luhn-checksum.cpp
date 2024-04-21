/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath/oath.h"

#include <QTest>
#include <QtDebug>

class LuhnChecksumTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testChecksum(void);
    void testChecksum_data(void);
};

static void define_test_case(quint32 value, uint digits, uint expected)
{
    static const QString testCase(QLatin1String("%1 (%2) ... %3"));
    QTest::newRow(qPrintable(testCase.arg(value).arg(digits).arg(expected))) << value << digits << expected;
}

void LuhnChecksumTest::testChecksum(void)
{
    QFETCH(quint32, value);
    QFETCH(uint, digits);
    QTEST(oath::luhnChecksum(value, digits), "expected");
}

void LuhnChecksumTest::testChecksum_data(void)
{
    QTest::addColumn<quint32>("value");
    QTest::addColumn<uint>("digits");
    QTest::addColumn<uint>("expected");

    define_test_case(0ULL, 1UL, 0UL);
    define_test_case(19ULL, 2ULL, 0ULL);
    define_test_case(1234567890ULL, 10UL, 3UL);
    define_test_case(987654321ULL, 10UL, 7UL);
}

QTEST_APPLESS_MAIN(LuhnChecksumTest)

#include "luhn-checksum.moc"
