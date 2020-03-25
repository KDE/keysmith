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

static void define_padding_test_cases(const QString &prefix, const QString &fixed, QValidator::State result, bool validEvery8th = false)
{
    QString input = prefix;
    for (int p = 0; p < 10; ++p) {
        input += QLatin1Char('=');
        if (validEvery8th && (input.size() % 8) == 0 && input.size() > 0) {
            continue;
        }
        define_test_case(input, fixed, result);
    }
}

static void define_padding_table(void)
{
    define_padding_test_cases(QLatin1String(""), QLatin1String(""), QValidator::Invalid);
    define_padding_test_cases(QLatin1String("V"), QLatin1String("V"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VV"), QLatin1String("VV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VA"), QLatin1String("VA======"), QValidator::Intermediate, true);
    define_padding_test_cases(QLatin1String("VVV"), QLatin1String("VVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVV"), QLatin1String("VVVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVA"), QLatin1String("VVVA===="), QValidator::Intermediate, true);
    define_padding_test_cases(QLatin1String("VVVVV"), QLatin1String("VVVVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVVA"), QLatin1String("VVVVA==="), QValidator::Intermediate, true);
    define_padding_test_cases(QLatin1String("VVVVVV"), QLatin1String("VVVVVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVVVVV"), QLatin1String("VVVVVVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVVVVA"), QLatin1String("VVVVVVA="), QValidator::Intermediate, true);
    define_test_case(QLatin1String("VVVVVVVV"), QLatin1String("VVVVVVVV"), QValidator::Acceptable);
    define_padding_test_cases(QLatin1String("VVVVVVVV="), QLatin1String("VVVVVVVV"), QValidator::Intermediate);

    // check things still work when upgrading to secrets > 8 characters long ;)
    define_padding_test_cases(QLatin1String("VVVVVVVVV"), QLatin1String("VVVVVVVVV"), QValidator::Intermediate);
    define_padding_test_cases(QLatin1String("VVVVVVVVVA"), QLatin1String("VVVVVVVVVA======"), QValidator::Intermediate, true);
}

static void define_conversion_table(void)
{
    /*
     * check that case conversion is applied for better UX
     * check that leading and trailing whitespace is stripped for better UX
     */
    define_test_case(QLatin1String(" v"), QLatin1String("V"), QValidator::Invalid);
    define_test_case(QLatin1String("va  "), QLatin1String("VA======"), QValidator::Invalid);
    define_test_case(QLatin1String("\tkeybytes\n "), QLatin1String("KEYBYTES"), QValidator::Invalid);
    define_test_case(QLatin1String("key \t\r\nbytes"), QLatin1String("KEYBYTES"), QValidator::Invalid);
    define_test_case(QLatin1String("\t\n\r value===\r\t \n"), QLatin1String("VALUE==="), QValidator::Invalid);
    define_test_case(QLatin1String("\t\n\r value \t\r\n===\r\t \n"), QLatin1String("VALUE==="), QValidator::Invalid);
}

static void define_valid_table(void)
{
    // check that some random valid keys are accepted
    define_test_case(QLatin1String("KEYBYTES"), QLatin1String("KEYBYTES"), QValidator::Acceptable);
    define_test_case(QLatin1String("VALUE==="), QLatin1String("VALUE==="), QValidator::Acceptable);
    define_test_case(QLatin1String("KEYBYTES=="), QLatin1String("KEYBYTES"), QValidator::Intermediate);
}

static void define_empty_table()
{
    define_test_case(QLatin1String(""), QLatin1String(""), QValidator::Intermediate);
    define_test_case(QLatin1String("  "), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\t"), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), QValidator::Invalid);
}

static void define_data(void)
{
    define_test_data_columns();
    define_empty_table();
    define_padding_table();
    define_conversion_table();
    define_valid_table();

    // these cases should be 'caught' and prohibited by the regex pattern
    // expected that fixup() logic is not applied
    define_test_case(QLatin1String("\r\n="), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String(" \t== \n"), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("INVALID1"), QLatin1String("INVALID1"), QValidator::Invalid);
    define_test_case(QLatin1String("=INVALID"), QLatin1String("=INVALID"), QValidator::Invalid);
    define_test_case(QLatin1String("INV= LID"), QLatin1String("INV=LID"), QValidator::Invalid);
    define_test_case(QLatin1String("INVA=LID"), QLatin1String("INVA=LID"), QValidator::Invalid);
    define_test_case(QLatin1String("WILLNOTFIXTHIS1"), QLatin1String("WILLNOTFIXTHIS1"), QValidator::Invalid);
}

DEFINE_VALIDATOR_TEST(Base32ValidatorTest, validators::Base32Validator, define_data);

QTEST_APPLESS_MAIN(Base32ValidatorTest)

#include "base32-validator.moc"
