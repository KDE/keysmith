/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "test-util.h"

#include "validators/countervalidator.h"

using namespace validators::test;

static QLocale frFR(QLocale::French, QLocale::France);
static QLocale nlNL(QLocale::Dutch, QLocale::Netherlands);
static QLocale enUK(QLocale::English, QLocale::UnitedKingdom);

/*
 * French is weird.
 * Or at least the QLocale for France is definitely inconsistent.
 * Sometimes you get 123\u00A0456, sometimes you get 123\202f\456...
 */
static const QString french = frFR.toString(123456ULL);
static const QString french2 = frFR.toString(12345ULL);

static void define_valid_table(void)
{
    define_test_case(QLatin1String("42"), QLatin1String("42"), QValidator::Acceptable);
    define_test_case(QLatin1String("\r \n\t42\r\t \n"), QLatin1String("42"), QValidator::Acceptable);

    define_test_case(QLatin1String("33  "), QLatin1String("33"), QValidator::Acceptable);
    define_test_case(QLatin1String("2 2"), QLatin1String("22"), QValidator::Invalid);
    define_test_case(QLatin1String("2\t\r\n2"), QLatin1String("22"), QValidator::Invalid);

    define_test_case(QLatin1String("123,456"), QLatin1String("123456"), QValidator::Acceptable, QLocale::c());

    define_test_case(QLatin1String("123456"), QLatin1String("123,456"), QValidator::Acceptable, enUK);
    define_test_case(QLatin1String("123,456"), QLatin1String("123,456"), QValidator::Acceptable, enUK);
    define_test_case(QLatin1String("123 456"), QLatin1String("123,456"), QValidator::Invalid, enUK);

    define_test_case(QLatin1String("123456"), QLatin1String("123.456"), QValidator::Acceptable, nlNL);
    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), QValidator::Acceptable, nlNL);
    define_test_case(QLatin1String("123 456"), QLatin1String("123.456"), QValidator::Invalid, nlNL);
    define_test_case(QLatin1String("123,456"), QLatin1String("123.456"), QValidator::Acceptable, nlNL);

    define_test_case(french, french, QValidator::Acceptable, frFR);
    define_test_case(QLatin1String("123456"), french, QValidator::Acceptable, frFR);
    define_test_case(QLatin1String("123,456"), french, QValidator::Acceptable, frFR);
    define_test_case(QLatin1String("123 456"), french, QValidator::Acceptable, frFR);
    define_test_case(QLatin1String("123 45"), french2, QValidator::Invalid, frFR);
}

static void define_empty_table(void)
{
    define_test_case(QLatin1String(""), QLatin1String(""), QValidator::Intermediate);
    define_test_case(QLatin1String("  "), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\t"), QLatin1String(""), QValidator::Invalid);
    define_test_case(QLatin1String("\r\n"), QLatin1String(""), QValidator::Invalid);
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

    define_test_case(QLatin1String("-33"), QLatin1String("-33"), QValidator::Invalid);
    define_test_case(QLatin1String("ZZ9 Plural Z Alpha"), QLatin1String("ZZ9PluralZAlpha"), QValidator::Invalid);

    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), QValidator::Invalid, frFR);
    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), QValidator::Invalid, enUK);
    define_test_case(QLatin1String("123.456"), QLatin1String("123.456"), QValidator::Invalid, QLocale::c());
    define_test_case(french, french, QValidator::Invalid, enUK);
    define_test_case(french, french, QValidator::Invalid, nlNL);
}

DEFINE_VALIDATOR_TEST(UnsignedLongValidatorTest, validators::UnsignedLongValidator, define_data);

QTEST_APPLESS_MAIN(UnsignedLongValidatorTest)

#include "unsigned-long-validator.moc"
