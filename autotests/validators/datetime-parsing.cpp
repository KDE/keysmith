/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "validators/datetimevalidator.h"

#include <QDateTime>
#include <QObject>
#include <QTest>
#include <QValidator>
#include <QtDebug>

Q_DECLARE_METATYPE(std::optional<QDateTime>);

class DateTimeParsingSamplesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParseDateTime(void);
    void testParseDateTime_data(void);
};

void DateTimeParsingSamplesTest::testParseDateTime(void)
{
    QFETCH(QString, input);
    QTEST(validators::parseDateTime(input), "result");
}

static void define_test_case(const QString &input, const std::optional<QDateTime> &result)
{
    QTest::newRow(qPrintable(input)) << input << result;
}

void DateTimeParsingSamplesTest::testParseDateTime_data(void)
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<std::optional<QDateTime>>("result");

    define_test_case(QLatin1String("1970-01-01T00:00:00Z"), std::optional<QDateTime>(QDateTime::fromMSecsSinceEpoch(0)));
    define_test_case(QLatin1String("1970-01-01T00:00:00.500Z"), std::optional<QDateTime>(QDateTime::fromMSecsSinceEpoch(500LL)));
    define_test_case(QLatin1String("01-01-1970 01:00"), std::nullopt);
}

QTEST_APPLESS_MAIN(DateTimeParsingSamplesTest)

#include "datetime-parsing.moc"
