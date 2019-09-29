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

#include "validators/secretvalidator.h"

#include <QTest>
#include <QValidator>
#include <QtDebug>

Q_DECLARE_METATYPE(QValidator::State);

class Base32ValidatorTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFixup(void);
    void testFixup_data(void);
    void testValidate(void);
    void testValidate_data(void);
};

static void define_test_data_columns(void)
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("fixed");
    QTest::addColumn<int>("cursor");
    QTest::addColumn<QValidator::State>("result");
}

static void define_test_case(const QString &input, const QString &fixed, int cursor=0, QValidator::State result=QValidator::Intermediate)
{
    QTest::newRow(qPrintable(input)) << input << fixed << cursor << result;
}

static void define_padding_test_cases(const QString &prefix, const QString &fixed, QValidator::State result)
{
    QString input = prefix;
    int prefixSize = prefix.size();
    for (int p = 0; p < 10; ++p) {
        input += QLatin1Char('=');
        int cursor = prefix.size();

        if (result == QValidator::Acceptable) {
            cursor = input.size();

            // expect the excess padding to be trimmed and the cursor to be
            // moved accordingly
            int cap = (prefixSize % 8) == 0
                ? prefixSize
                : (8 * (1 + prefixSize / 8));
            if (cursor > cap) {
                cursor = cap;
            }
        }

        define_test_case(input, fixed, cursor, result);
    }
}

static void define_padding_table(void)
{
    define_padding_test_cases(QLatin1String("V"), QLatin1String("V"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VV"), QLatin1String("VV======"), QValidator::Acceptable);
    define_padding_test_cases(QLatin1String("VVV"), QLatin1String("VVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVV"), QLatin1String("VVVV===="), QValidator::Acceptable);
    define_padding_test_cases(QLatin1String("VVVVV"), QLatin1String("VVVVV==="), QValidator::Acceptable);
    define_padding_test_cases(QLatin1String("VVVVVV"), QLatin1String("VVVVVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVVVVV"), QLatin1String("VVVVVVV="), QValidator::Acceptable);
    define_padding_test_cases(QLatin1String("VVVVVVVV"), QLatin1String("VVVVVVVV"), QValidator::Acceptable);

    // check things still work when upgrading to secrets > 8 characters long ;)
    define_padding_test_cases(QLatin1String("VVVVVVVVV"), QLatin1String("VVVVVVVVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVVVVVVVV"), QLatin1String("VVVVVVVVVV======"), QValidator::Acceptable);
}

static void define_conversion_table(void)
{
    /*
     * check that case conversion is applied for better UX
     * check that leading and trailing whitespace is stripped for better UX
     */
    define_test_case(QLatin1String(" v"), QLatin1String("V"), 1, QValidator::Intermediate);
    define_test_case(QLatin1String("vv  "), QLatin1String("VV======"), 2, QValidator::Acceptable);
    define_test_case(QLatin1String("\tkeybytes\n "), QLatin1String("KEYBYTES"), 8, QValidator::Acceptable);
    define_test_case(QLatin1String("key \t\r\nbytes"), QLatin1String("KEYBYTES"), 8, QValidator::Acceptable);
    define_test_case(QLatin1String("\t\n\r valid===\r\t \n"), QLatin1String("VALID==="), 8, QValidator::Acceptable);
    define_test_case(QLatin1String("\t\n\r valid \t\r\n===\r\t \n"), QLatin1String("VALID==="), 8, QValidator::Acceptable);
}

static void define_valid_table(void)
{
    // check that some random valid keys are accepted
    define_test_case(QLatin1String("KEYBYTES"), QLatin1String("KEYBYTES"), 8, QValidator::Acceptable);
    define_test_case(QLatin1String("VALID==="), QLatin1String("VALID==="), 8, QValidator::Acceptable);
    define_test_case(QLatin1String("KEYBYTES=="), QLatin1String("KEYBYTES"), 8, QValidator::Acceptable);
}

static void define_empty_table()
{
    define_test_case(QLatin1String(""), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("  "), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("\t"), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), 0, QValidator::Intermediate);
}

void Base32ValidatorTest::testFixup(void)
{
    const validators::Base32Validator uut;
    QFETCH(QString, input);

    uut.fixup(input);
    QTEST(input, "fixed");

    // test if output is stable
    // ie.: fixup(fixup()) == fixup()
    uut.fixup(input);
    QTEST(input, "fixed");
}

void Base32ValidatorTest::testValidate(void)
{
    const validators::Base32Validator uut;

    QFETCH(QString, input);
    QFETCH(QString, fixed);
    QFETCH(int, cursor);

    int position = input.size();

    QTEST(uut.validate(input, position), "result");
    QCOMPARE(input, fixed);
    QCOMPARE(position, cursor);
}

void Base32ValidatorTest::testValidate_data(void)
{
    define_test_data_columns();
    define_empty_table();
    define_padding_table();
    define_conversion_table();
    define_valid_table();


    // these cases should be 'caught' and prohibited by the regex pattern
    // expected that fixup() logic is not applied during validation
    define_test_case(QLatin1String("="), QLatin1String("="), 1, QValidator::Invalid);
    define_test_case(QLatin1String("=="), QLatin1String("=="), 2, QValidator::Invalid);
    define_test_case(QLatin1String("\r\n="), QLatin1String("="), 1, QValidator::Invalid);
    define_test_case(QLatin1String(" \t== \n"), QLatin1String("=="), 2, QValidator::Invalid);
    define_test_case(QLatin1String("INVALID1"), QLatin1String("INVALID1"), 8, QValidator::Invalid);
    define_test_case(QLatin1String("=INVALID"), QLatin1String("=INVALID"), 8, QValidator::Invalid);
    define_test_case(QLatin1String("INV= LID"), QLatin1String("INV=LID"), 7, QValidator::Invalid);
    define_test_case(QLatin1String("INVA=LID"), QLatin1String("INVA=LID"), 8, QValidator::Invalid);
    define_test_case(QLatin1String("WILLNOTFIXTHIS1"), QLatin1String("WILLNOTFIXTHIS1"), 15, QValidator::Invalid);
}

void Base32ValidatorTest::testFixup_data(void)
{
    define_test_data_columns();
    define_empty_table();
    define_padding_table();
    define_conversion_table();
    define_valid_table();

    // check various no data, just padding cases
    define_test_case(QLatin1String("="), QLatin1String(""));
    define_test_case(QLatin1String("=="), QLatin1String(""));
    define_test_case(QLatin1String("==="), QLatin1String(""));
    define_test_case(QLatin1String("===="), QLatin1String(""));
    define_test_case(QLatin1String("====="), QLatin1String(""));
    define_test_case(QLatin1String("======"), QLatin1String(""));
    define_test_case(QLatin1String("======="), QLatin1String(""));
    define_test_case(QLatin1String("========"), QLatin1String(""));
    define_test_case(QLatin1String("========="), QLatin1String(""));
}

QTEST_APPLESS_MAIN(Base32ValidatorTest)

#include "base32-validator.moc"

