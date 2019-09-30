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

#include "validators/countervalidator.h"

using namespace validators::test;

static QLocale frFR(QLocale::French, QLocale::France);
static QLocale nlNL(QLocale::Dutch, QLocale::Netherlands);
static QLocale enUK(QLocale::English, QLocale::UnitedKingdom);

static const QString french = QString::fromWCharArray(L"123\u00A0456");

static void define_valid_table(void)
{
    define_test_case(QLatin1String("42"), QLatin1String("42"), 2, QValidator::Acceptable);
    define_test_case(QLatin1String("\r \n\t42\r\t \n"), QLatin1String("42"), 2, QValidator::Acceptable);

    define_test_case(QLatin1String("33  "), QLatin1String("33"), 2, QValidator::Acceptable);
    define_test_case(QLatin1String("2 2"), QLatin1String("22"), 2, QValidator::Acceptable);
    define_test_case(QLatin1String("2\t\r\n2"), QLatin1String("22"), 2, QValidator::Acceptable);

    define_test_case(QLatin1String("123,456"), QLatin1String("123456"), 6, QValidator::Acceptable, QLocale::c());

    define_test_case(QLatin1String("123456"), QLatin1String("123,456"), 7, QValidator::Acceptable, enUK);
    define_test_case(QLatin1String("123,456"), QLatin1String("123,456"), 7, QValidator::Acceptable, enUK);
    define_test_case(QLatin1String("123 456"), QLatin1String("123,456"), 7, QValidator::Acceptable, enUK);

    define_test_case(QLatin1String("123456"), QLatin1String("123.456"), 7, QValidator::Acceptable, nlNL);
    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), 7, QValidator::Acceptable, nlNL);
    define_test_case(QLatin1String("123 456"), QLatin1String("123.456"), 7, QValidator::Acceptable, nlNL);
    define_test_case(QLatin1String("123,456"), QLatin1String("123.456"), 7, QValidator::Acceptable, nlNL);

    define_test_case(french, french, 7, QValidator::Acceptable, frFR);
    define_test_case(QLatin1String("123456"), french, 7, QValidator::Acceptable, frFR);
    define_test_case(QLatin1String("123,456"), french, 7, QValidator::Acceptable, frFR);
    define_test_case(QLatin1String("123 456"), french, 7, QValidator::Acceptable, frFR);
}

static void define_empty_table(void)
{
    define_test_case(QLatin1String(""), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("  "), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("\t"), QLatin1String(""), 0, QValidator::Intermediate);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), 0, QValidator::Intermediate);
}

/*
 * Due to how the UnsignedLongValidator works fixup() is always called and fixup()
 * never yields outright invalid output
 *
 * As a result it is not needed to maintain separate input samples for
 * both fixup() and validate().
 */

static void define_data(void)
{
    define_empty_table();
    define_valid_table();

    define_test_case(QLatin1String("-33"), QLatin1String("-33"), 3, QValidator::Invalid);
    define_test_case(QLatin1String("ZZ9 Plural Z Alpha"), QLatin1String("ZZ9PluralZAlpha"), 15, QValidator::Invalid);

    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), 7, QValidator::Invalid, frFR);
    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), 7, QValidator::Invalid, enUK);
    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), 7, QValidator::Invalid, QLocale::c());
    define_test_case(french, french, 7, QValidator::Invalid, enUK);
    define_test_case(french, french, 7, QValidator::Invalid, nlNL);
}

DEFINE_VALIDATOR_TEST(UnsignedLongValidatorTest, validators::UnsignedLongValidator, define_data, define_data);

QTEST_APPLESS_MAIN(UnsignedLongValidatorTest)

#include "unsigned-long-validator.moc"
