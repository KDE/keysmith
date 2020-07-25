/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "validators/countervalidator.h"

#include <QObject>
#include <QTest>
#include <QValidator>
#include <QtDebug>

Q_DECLARE_METATYPE(std::optional<qulonglong>);
Q_DECLARE_METATYPE(QLocale);

class UnsignedLongParsingSamplesTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParseUnsignedInteger(void);
    void testParseUnsignedInteger_data(void);
};


void UnsignedLongParsingSamplesTest::testParseUnsignedInteger(void)
{
    QFETCH(QString, input);
    QFETCH(QLocale, locale);
    QTEST(validators::parseUnsignedInteger(input, locale), "result");
}

static void define_test_case(const QString &input, const QLocale &locale, const std::optional<qulonglong> &result)
{
    QTest::newRow(qPrintable(input)) << input << locale << result;
}

void UnsignedLongParsingSamplesTest::testParseUnsignedInteger_data(void)
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QLocale>("locale");
    QTest::addColumn<std::optional<qulonglong>>("result");

    QLocale frFR(QLocale::French, QLocale::France);
    QLocale nlNL(QLocale::Dutch, QLocale::Netherlands);
    QLocale enUK(QLocale::English, QLocale::UnitedKingdom);

    define_test_case(QLatin1String("123456"), frFR, std::optional<qulonglong>(123'456ULL));
    define_test_case(QLatin1String("123 456"), frFR, std::optional<qulonglong>(123'456ULL));

    define_test_case(QLatin1String("12 3456"), frFR, std::nullopt);
    define_test_case(QLatin1String("1234 56"), frFR, std::nullopt);
    define_test_case(QLatin1String("12345 6"), frFR, std::nullopt);

    define_test_case(QLatin1String("123.456"), nlNL, std::optional<qulonglong>(123'456ULL));
    define_test_case(QLatin1String("123,456"), enUK, std::optional<qulonglong>(123'456ULL));
}

QTEST_APPLESS_MAIN(UnsignedLongParsingSamplesTest)

#include "unsigned-long-parsing.moc"
