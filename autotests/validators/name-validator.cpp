/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "test-util.h"

#include "validators/namevalidator.h"

using namespace validators::test;

static void define_valid_table(void)
{
    define_test_case(QLatin1String("Outis"), QLatin1String("Outis"), QValidator::Acceptable);
    define_test_case(QLatin1String("test account"), QLatin1String("test account"), QValidator::Acceptable);
}

static void define_invalid_data(void)
{
    define_test_case(QLatin1String("test\taccount"), QLatin1String("test account"), QValidator::Invalid);
    define_test_case(QLatin1String("\r \n\ttest\r\t \naccount \r\t\n"), QLatin1String("test account"), QValidator::Invalid);
    define_test_case(QLatin1String("test  "), QLatin1String("test "), QValidator::Invalid);
}

static void define_empty_table(void)
{
    define_test_case(QLatin1String(""), QLatin1String(""), QValidator::Intermediate);
    define_test_case(QLatin1String("  "), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\t"), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), QValidator::Invalid);
}

static void define_data(void)
{
    define_empty_table();
    define_valid_table();
    define_invalid_data();
}

DEFINE_VALIDATOR_TEST(NameValidatorTest, validators::NameValidator, define_data);

QTEST_APPLESS_MAIN(NameValidatorTest)

#include "name-validator.moc"
