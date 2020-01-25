/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "base32/base32.h"

#include <QTest>
#include <QtDebug>

class Base32CodingDecodingTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSample(void);
    void testSample_data(void);
};

static int lastPadBits(int data)
{
    switch (data) {
    case 7:
        return 3;
    case 5:
        return 1;
    case 4:
        return 4;
    case 2:
        return 2;
    default:
        return 0;
    }
}

static int outputSize(int data)
{
    switch (data) {
    case 8:
        return 5;
    case 7:
        return 4;
    case 5:
        return 3;
    case 4:
        return 2;
    case 2:
        return 1;
    default:
        return 0;
    }
}

static void define_test_data(void)
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QByteArray>("expected");
}

static void define_test_case(const QString &input, int len, char value)
{
    static const QString testCase(QLatin1String("size: %1: '%2' ... 0x%3"));
    QByteArray expected;
    int outputSz = outputSize(len);
    expected.reserve(outputSz);
    expected.resize(outputSz);
    expected.fill('\x0');
    if (len > 0) {
        expected[expected.size() - 1] = value;
    }

    QTest::newRow(qPrintable(testCase.arg(len).arg(input).arg(QLatin1String(expected.toHex())))) << input << expected;
}

static inline QChar pick(int v)
{
    return v < 26 ? QLatin1Char('A' + v) : QLatin1Char('2' + v - 26);
}

static void define_test_case(int len)
{
    QString prefix;
    QByteArray output;
    int padBits = lastPadBits(len);
    for(int i = 3; i < len; ++i) {
        prefix += QLatin1Char('A');
    }

    for (int b = 0; b < 256; ++b) {
        int i1 = ((b << padBits) >> 10) & 0x1F;
        int i2 = (b >> (5 - padBits)) & 0x1F;
        int i3 = (b << padBits) & 0x1F;

        QString input = prefix;
        if (len >= 3) {
            input += pick(i1);
        }
        input += pick(i2);
        input += pick(i3);

        while(input.size() < 8) {
            input += QLatin1Char('=');
        }
        define_test_case(input, len, b);
    }
}

void Base32CodingDecodingTest::testSample(void)
{
    QFETCH(QString, input);
    QFETCH(QByteArray, expected);

    QByteArray work(expected.size(), '\x0');

    QCOMPARE(base32::decode(input, work.data(), work.size()), std::optional<size_t>(expected.size()));
    QCOMPARE(work, expected);
}

void Base32CodingDecodingTest::testSample_data(void)
{
    define_test_data();
    QTest::newRow(qPrintable(QLatin1String("the empty string"))) << QString(QLatin1String("")) << QByteArray();
    define_test_case(2);
    define_test_case(4);
    define_test_case(5);
    define_test_case(7);
    define_test_case(8);
}

QTEST_APPLESS_MAIN(Base32CodingDecodingTest)

#include "base32-coding-decoding.moc"
