/*****************************************************************************
 * Copyright: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>                  *
 *                                                                           *
 * This project is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This project is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

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
    void testParsea(void);
    void testParsea_data(void);
};


void UnsignedLongParsingSamplesTest::testParsea(void)
{
    QFETCH(QString, input);
    QFETCH(QLocale, locale);
    QTEST(validators::parse(input, locale), "result");
}

static void define_test_case(const QString &input, const QLocale &locale, const std::optional<qulonglong> &result)
{
    QTest::newRow(qPrintable(input)) << input << locale << result;
}

void UnsignedLongParsingSamplesTest::testParsea_data(void)
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
