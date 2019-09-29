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

#include "test-util.h"

#include "validators/secretvalidator.h"

using namespace validators::test;

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

static void define_validate_data(void)
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

static void define_fixup_data(void)
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

DEFINE_VALIDATOR_TEST(Base32ValidatorTest, validators::Base32Validator, define_fixup_data, define_validate_data);

QTEST_APPLESS_MAIN(Base32ValidatorTest)

#include "base32-validator.moc"
