/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "test-util.h"

#include "validators/issuervalidator.h"

using namespace validators::test;

static void define_valid_table(void)
{
    define_test_case(QLatin1String(""), QLatin1String(""), QValidator::Acceptable);
    define_test_case(QLatin1String("Issuer"), QLatin1String("Issuer"), QValidator::Acceptable);
    define_test_case(QLatin1String("test issuer"), QLatin1String("test issuer"), QValidator::Acceptable);
}

static void define_invalid_table(void)
{
    define_test_case(QLatin1String("test\tissuer"), QLatin1String("test issuer"), QValidator::Invalid);
    define_test_case(QLatin1String("\r \n\ttest\r\t \nissuer \r\t\n"), QLatin1String("test issuer"), QValidator::Invalid);
    define_test_case(QLatin1String("test  "), QLatin1String("test "), QValidator::Invalid);
    define_test_case(QLatin1String("test:issuer"), QLatin1String("testissuer"), QValidator::Invalid);
}

static void define_empty_table(void)
{
    define_test_case(QLatin1String("  "), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\t"), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), QValidator::Invalid);
}

static void define_data(void)
{
    define_empty_table();
    define_valid_table();
    define_invalid_table();
}

DEFINE_VALIDATOR_TEST(IssuerValidatorTest, validators::IssuerValidator, define_data);

QTEST_APPLESS_MAIN(IssuerValidatorTest)

#include "issuer-validator.moc"
