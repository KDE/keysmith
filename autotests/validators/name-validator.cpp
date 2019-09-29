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

#include "validators/namevalidator.h"

using namespace validators::test;

static void define_valid_table(void)
{
    define_test_case(QLatin1String("Outis"), QLatin1String("Outis"), 5, QValidator::Acceptable);
    define_test_case(QLatin1String("test\taccount"), QLatin1String("test account"), 12, QValidator::Acceptable);
    define_test_case(QLatin1String("\r \n\ttest\r\t \naccount \r\t\n"), QLatin1String("test account"), 12, QValidator::Acceptable);
}

static void define_empty_table(void)
{
    define_test_case(QLatin1String(""), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("  "), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("\t"), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), 0, QValidator::Intermediate);
}

/*
 * Due to how the NameValidator works fixup() is always called and fixup()
 * never yields outright invalid output (it strips spaces, the only way
 * to get invalid output is due to misplaced spaces)
 *
 * As a result it is not needed to maintain separate input samples for
 * both fixup() and validate().
 */

static void define_data(void)
{
    define_empty_table();
    define_valid_table();

    define_test_case(QLatin1String("test  "), QLatin1String("test "), 5, QValidator::Intermediate);
}

DEFINE_VALIDATOR_TEST(NameValidatorTest, validators::NameValidator, define_data, define_data);

QTEST_APPLESS_MAIN(NameValidatorTest)

#include "name-validator.moc"
