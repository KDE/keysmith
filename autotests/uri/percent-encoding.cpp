/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "uri/uri.h"

#include <QTest>
#include <QVector>
#include <QtDebug>

class PercentEncodingTest: public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void testValidString(void);
    void testValidString_data(void);
    void testValidByteArray(void);
    void testValidByteArray_data(void);
    void testInvalidString(void);
    void testInvalidString_data(void);
    void testInvalidByteArray(void);
    void testInvalidByteArray_data(void);
};

void PercentEncodingTest::testValidString(void)
{
    QFETCH(QByteArray, input);
    const std::optional<QString> result = uri::decodePercentEncoding(input);
    QVERIFY2(result, "should decode valid input successfully");
    QTEST(*result, "expected");
}

void PercentEncodingTest::testValidString_data(void)
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QString>("expected");
    QVector<QByteArray> validStringInputs = QVector<QByteArray>() << QByteArray("%3A");
    QStringList validStringOutputs = QStringList() << QStringLiteral(":");
    int i = 0;
    for (const auto &input : qAsConst(validStringInputs)) {
        QTest::newRow(input.constData()) << input << validStringOutputs[i];
        i++;
    }
}

void PercentEncodingTest::testValidByteArray(void)
{
    QFETCH(QByteArray, input);
    const std::optional<QByteArray> result = uri::fromPercentEncoding(input);
    QVERIFY2(result, "should decode valid input successfully");
    QTEST(*result, "expected");
}

void PercentEncodingTest::testValidByteArray_data(void)
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("expected");

    QVector<QByteArray> validByteArrayInputs = QVector<QByteArray>()
        << QByteArray("%01")
        << QByteArray("%3A")
        << QByteArray("%00")
        << QByteArray("a%20valid%20sample")
        << QByteArray("%2f")
        << QByteArray("embedded%00works");

    QVector<QByteArray> validByteArrayOutputs = QVector<QByteArray>()
        << QByteArray("\x1")
        << QByteArray(":")
        << QByteArray("Z").replace('Z', '\0')
        << QByteArray("a valid sample")
        << QByteArray("/")
        << QByteArray("embeddedZworks").replace('Z', '\0');

    int i = 0;
    for (const auto &input : qAsConst(validByteArrayInputs)) {
        QTest::newRow(input.constData()) << input << validByteArrayOutputs[i];
        i++;
    }
}

void PercentEncodingTest::testInvalidString(void)
{
    QFETCH(QByteArray, input);
    QVERIFY2(!uri::decodePercentEncoding(input), "should reject invalid input");
}

void PercentEncodingTest::testInvalidString_data(void)
{
    QTest::addColumn<QByteArray>("input");
    QVector<QByteArray> invalidStringInputs = QVector<QByteArray>()
        << QByteArray("%ff broken multibyte (no 0 in leading char)")
        << QByteArray("%cf broken multibyte (next char not marked)")
        << QByteArray("%c0%7f broken multibyte (over long)")
        << QByteArray("truncated multibyte %c0");
    for (const auto &input : std::as_const(invalidStringInputs)) {
        QTest::newRow(input.constData()) << input;
    }
}

void PercentEncodingTest::testInvalidByteArray(void)
{
    QFETCH(QByteArray, input);
    QVERIFY2(!uri::fromPercentEncoding(input), "should reject invalid input");
}

void PercentEncodingTest::testInvalidByteArray_data(void)
{
    QTest::addColumn<QByteArray>("input");
    QVector<QByteArray> invalidByteArrayInputs = QVector<QByteArray>()
        << QByteArray("%")
        << QByteArray("invalid%")
        << QByteArray("%G5")
        << QByteArray("%5");
    for (const auto &input : qAsConst(invalidByteArrayInputs)) {
        QTest::newRow(input.constData()) << input;
    }
}

QTEST_APPLESS_MAIN(PercentEncodingTest)

#include "percent-encoding.moc"
