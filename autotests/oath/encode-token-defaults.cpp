/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath/oath.h"

#include <QTest>
#include <QtDebug>

class DefaultEncoderTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void encode(void);
    void encode_data(void);
};

static void define_test_case(quint32 value, uint tokenLength, bool checksum, const QString &expected)
{
    static const QString testCase(QLatin1String("%1 (%2, %3) ... %4"));
    QTest::newRow(qPrintable(testCase.arg(value).arg(tokenLength).arg(checksum ? QLatin1String("with checksum") : QLatin1String("without checksum")).arg(expected)))
        << value << checksum << tokenLength << expected;
}

void DefaultEncoderTest::encode(void)
{
    QFETCH(quint32, value);
    QFETCH(bool, checksum);
    QFETCH(uint, tokenLength);

    oath::Encoder uut(tokenLength, checksum);
    QTEST(uut.encode(value), "expected");
}

void DefaultEncoderTest::encode_data(void)
{
    QTest::addColumn<quint32>("value");
    QTest::addColumn<bool>("checksum");
    QTest::addColumn<uint>("tokenLength");
    QTest::addColumn<QString>("expected");

    define_test_case(23, 6, false, QLatin1String("000023"));
    define_test_case(23, 6, true, QLatin1String("0000232"));
    define_test_case(151'230'000, 9, false, QLatin1String("151230000"));
    define_test_case(151'230'000, 9, true, QLatin1String("1512300003"));
    define_test_case(175'230'000, 7, false, QLatin1String("5230000"));
    define_test_case(175'230'000, 7, true, QLatin1String("52300001"));
    define_test_case(2'000'000'009, 10, false, QLatin1String("2000000009"));
    define_test_case(2'000'000'009, 10, true, QLatin1String("20000000099"));
}

QTEST_APPLESS_MAIN(DefaultEncoderTest)

#include "encode-token-defaults.moc"
