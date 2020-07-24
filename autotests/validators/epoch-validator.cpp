/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "test-util.h"

#include "validators/datetimevalidator.h"

using namespace validators::test;

static void define_valid_table(void)
{
    define_test_case(QLatin1String("1970-01-01T00:00:00Z"), QLatin1String("1970-01-01T00:00:00Z"), QValidator::Acceptable);
    define_test_case(QLatin1String("1970-01-01T00:00:00.500Z"), QLatin1String("1970-01-01T00:00:00.500Z"), QValidator::Acceptable);
    define_test_case(QLatin1String("1969-12-31T23:05:00-01:00"), QLatin1String("1969-12-31T23:05:00-01:00"), QValidator::Acceptable);
    define_test_case(QLatin1String("1970-01-01T01:05:00+01:00"), QLatin1String("1970-01-01T01:05:00+01:00"), QValidator::Acceptable);
}

static void define_invalid_table(void)
{
    define_test_case(QLatin1String("Not a datetime"), QLatin1String("Not a datetime"), QValidator::Intermediate);
    define_test_case(QLatin1String("1980-01-01T00:00:00Z"), QLatin1String("1980-01-01T00:00:00Z"), QValidator::Invalid);
    define_test_case(QLatin1String("1969-12-31T23:05:00.001-01:00"), QLatin1String("1969-12-31T23:05:00.001-01:00"), QValidator::Invalid);
    define_test_case(QLatin1String("1970-01-01T01:05:00.001+01:00"), QLatin1String("1970-01-01T01:05:00.001+01:00"), QValidator::Invalid);
}

static void define_empty_table(void)
{
    define_test_case(QLatin1String(""), QLatin1String(""), QValidator::Intermediate);
    define_test_case(QLatin1String("  "), QLatin1String(""), QValidator::Intermediate);
    define_test_case(QLatin1String("\t"), QLatin1String(""), QValidator::Intermediate);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), QValidator::Intermediate);
}

static void define_data(void)
{
    define_empty_table();
    define_valid_table();
    define_invalid_table();
}

static qint64 fake_clock(void)
{
    return 300'000LL;
}

DEFINE_VALIDATOR_TEST(EpochValidatorTest, validators::EpochValidator, define_data, fake_clock);

QTEST_APPLESS_MAIN(EpochValidatorTest)

#include "epoch-validator.moc"
