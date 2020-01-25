/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "base32/base32.h"

#include <QTest>
#include <QtDebug>

class Base32ValidationTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidSample(void);
    void testValidSample_data(void);
    void testInvalidSample(void);
    void testInvalidSample_data(void);
};

static void define_valid_test_case(const QString &base32, size_t expected)
{
    static const QString testCase(QLatin1String("'%1'"));
    QTest::newRow(qPrintable(testCase.arg(base32))) << base32 << expected;
}

static void define_invalid_test_case(const char *testCase, const QString &input)
{
    QTest::newRow(qPrintable(testCase)) << input;
}

void Base32ValidationTest::testValidSample(void)
{
    QFETCH(QString, input);
    QFETCH(size_t, expected);
    QCOMPARE(base32::validate(input), std::optional<size_t>(expected));
}

void Base32ValidationTest::testInvalidSample(void)
{
    QFETCH(QString, input);
    QVERIFY2(!base32::validate(input), "invalid input should be rejected");
}

void Base32ValidationTest::testValidSample_data(void)
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<size_t>("expected");

    define_valid_test_case(QLatin1String("IFBEGRAK"), 5);
    define_valid_test_case(QLatin1String("GIYDCNQ="), 4);
    define_valid_test_case(QLatin1String("AAAQE==="), 3);
    define_valid_test_case(QLatin1String("HU6Q===="), 2);
    define_valid_test_case(QLatin1String("H4======"), 1);

    define_valid_test_case(QLatin1String(""), 0);
}

void Base32ValidationTest::testInvalidSample_data(void)
{
    QTest::addColumn<QString>("input");

    define_invalid_test_case("without any padding", QLatin1String("ZZ"));
    define_invalid_test_case("too little padding", QLatin1String("ZZ==="));
    define_invalid_test_case("padding only", QLatin1String("========"));
    define_invalid_test_case("embedded spaces", QLatin1String("ZZ \n===="));
    define_invalid_test_case("invalid base32 (1)", QLatin1String("1AABBCCD"));
    define_invalid_test_case("invalid base32 (8)", QLatin1String("AABBCC8D"));
    define_invalid_test_case("invalid base32 (@)", QLatin1String("AABBCCD@"));
}

QTEST_APPLESS_MAIN(Base32ValidationTest)

#include "base32-validate.moc"
