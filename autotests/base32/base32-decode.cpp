/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "base32/base32.h"

#include <QTest>
#include <QtDebug>

class Base32DecodingTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidSample(void);
    void testValidSample_data(void);
    void testInvalidSample(void);
    void testInvalidSample_data(void);
};

static void define_valid_test_case(const char *testCase, const char *data, int length, const QString &base32)
{
    QTest::newRow(qPrintable(QLatin1String(testCase))) << base32 << QByteArray(data, length);
}

static void define_invalid_test_case(const char *testCase, const QString &base32)
{
    QTest::newRow(qPrintable(QLatin1String(testCase))) << base32;
}

void Base32DecodingTest::testValidSample(void)
{
    QFETCH(QString, base32);
    QFETCH(QByteArray, expected);

    QByteArray work;
    work.reserve(expected.size());
    work.resize(expected.size());
    work.fill('\x0');

    QCOMPARE(base32::decode(base32, work.data(), work.size()), std::optional<size_t>(expected.size()));
    QCOMPARE(work, expected);

    QCOMPARE(base32::decode(base32), std::optional<QByteArray>(expected));
}

void Base32DecodingTest::testInvalidSample(void)
{
    QFETCH(QString, base32);

    QByteArray work;
    work.reserve(100);
    work.resize(100);
    work.fill('\x0');

    QCOMPARE(base32::decode(base32, work.data(), work.size()), std::nullopt);
    QCOMPARE(base32::decode(base32), std::nullopt);
}

void Base32DecodingTest::testValidSample_data(void)
{
    static const char ok_corpus[13][5] = {{'A', 'B', 'C', 'D', '\xA'},
                                          {'?', 'A', 'B', 'C', 'D'},
                                          {'2', '0', '1', '6'},
                                          {'=', '='},
                                          {'?'},
                                          {'8'},

                                          {'\x0', '\x1', '\x2'},
                                          {'\x1', '\x0', '\x2'},
                                          {'\x1', '\x2', '\x0'},

                                          {'\x0', 'A', 'B', '\x1', '\x2'},
                                          {'\x1', 'A', 'B', '\x0', '\x2'},
                                          {'\x1', 'A', 'B', '\x2', '\x0'},

                                          {}};

    QTest::addColumn<QString>("base32");
    QTest::addColumn<QByteArray>("expected");

    define_valid_test_case("'ABCD\\n'", ok_corpus[0], 5, QLatin1String("IFBEGRAK"));
    define_valid_test_case("'?ABCD'", ok_corpus[1], 5, QLatin1String("H5AUEQ2E"));
    define_valid_test_case("'2016'", ok_corpus[2], 4, QLatin1String("GIYDCNQ="));
    define_valid_test_case("'=='", ok_corpus[3], 2, QLatin1String("HU6Q===="));
    define_valid_test_case("'?'", ok_corpus[4], 1, QLatin1String("H4======"));
    define_valid_test_case("'8'", ok_corpus[5], 1, QLatin1String("HA======"));

    define_valid_test_case("'\\x0\\x1\\x2'", ok_corpus[6], 3, QLatin1String("AAAQE==="));
    define_valid_test_case("'\\x1\\x0\\x2'", ok_corpus[7], 3, QLatin1String("AEAAE==="));
    define_valid_test_case("'\\x1\\x2\\x0'", ok_corpus[8], 3, QLatin1String("AEBAA==="));

    define_valid_test_case("'\\x0AB\\x1\\x2'", ok_corpus[9], 5, QLatin1String("ABAUEAIC"));
    define_valid_test_case("'\\x1AB\\x0\\x2'", ok_corpus[10], 5, QLatin1String("AFAUEAAC"));
    define_valid_test_case("'\\x1AB\\x2\\x0'", ok_corpus[11], 5, QLatin1String("AFAUEAQA"));

    define_valid_test_case("''", ok_corpus[12], 0, QLatin1String(""));
}

void Base32DecodingTest::testInvalidSample_data(void)
{
    QTest::addColumn<QString>("base32");

    define_invalid_test_case("without any padding", QLatin1String("ZZ"));
    define_invalid_test_case("too little padding", QLatin1String("ZZ==="));
    define_invalid_test_case("padding only", QLatin1String("========"));
    define_invalid_test_case("embedded spaces", QLatin1String("ZZ \n===="));
    define_invalid_test_case("invalid base32 (1)", QLatin1String("1AABBCCD"));
    define_invalid_test_case("invalid base32 (8)", QLatin1String("AABBCC8D"));
    define_invalid_test_case("invalid base32 (@)", QLatin1String("AABBCCD@"));
}

QTEST_APPLESS_MAIN(Base32DecodingTest)

#include "base32-decode.moc"
