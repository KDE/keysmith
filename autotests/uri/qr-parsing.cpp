/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "uri/uri.h"

#include <QTest>
#include <QtDebug>

Q_DECLARE_METATYPE(uri::QrParts::Type);

class UriParsingTest: public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void testValid(void);
    void testValid_data(void);
    void testInvalid(void);
    void testInvalid_data(void);
};

static void define_valid_test_data(void)
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<uri::QrParts::Type>("type");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("issuer");
    QTest::addColumn<QString>("secret");
    QTest::addColumn<QString>("timeStep");
    QTest::addColumn<QString>("tokenLength");
    QTest::addColumn<QString>("algorithm");
    QTest::addColumn<QString>("counter");
}

static void define_valid_test_case(const char *testCase, const QString &input, uri::QrParts::Type type,
                                   const QString &name, const QString &issuer, const QString &secret,
                                   const QString &timeStep, const QString &tokenLength, const QString &algorithm,
                                   const QString &counter)
{
    QTest::newRow(qPrintable(testCase)) << input
        << type << name << issuer << secret << timeStep << tokenLength << algorithm << counter;
}

void UriParsingTest::testValid(void)
{
    QFETCH(QString, input);

    const std::optional<uri::QrParts> result = uri::QrParts::parse(input);
    QVERIFY2(result, "should parse valid input successfully");
    QTEST(result->type(), "type");
    QTEST(result->name(), "name");
    QTEST(result->issuer(), "issuer");
    QTEST(result->secret(), "secret");
    QTEST(result->tokenLength(), "tokenLength");
    QTEST(result->algorithm(), "algorithm");
    QTEST(result->counter(), "counter");
}

void UriParsingTest::testValid_data(void)
{
    define_valid_test_data();
    define_valid_test_case("hotp (all fields)",
                           QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42"),
                           uri::QrParts::Type::Hotp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QStringLiteral("30"), QStringLiteral("6"), QStringLiteral("sha1"),
                           QStringLiteral("42"));
    define_valid_test_case("hotp (all fields, URI encoded label separator)",
                           QStringLiteral("otpauth://hotp/issuer%3Ana:me?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42"),
                           uri::QrParts::Type::Hotp, QStringLiteral("na:me"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QStringLiteral("30"), QStringLiteral("6"), QStringLiteral("sha1"),
                           QStringLiteral("42"));
    define_valid_test_case("hotp (minimal)",
                           QStringLiteral("otpauth://hotp/name?secret=VALUE&counter=42"),
                           uri::QrParts::Type::Hotp, QStringLiteral("name"), QString(), QStringLiteral("VALUE"),
                           QString(), QString(), QString(), QStringLiteral("42"));
    define_valid_test_case("hotp (minimal, without name and missing params)",
                           QStringLiteral("otpauth://hotp?secret=VALUE"),
                           uri::QrParts::Type::Hotp, QString(), QString(), QStringLiteral("VALUE"), QString(),
                           QString(), QString(), QString());
    define_valid_test_case("hotp (issuer only in label)",
                           QStringLiteral("otpauth://hotp/issuer:name?secret=VALUE&counter=42"),
                           uri::QrParts::Type::Hotp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QString(), QString(), QString(), QStringLiteral("42"));
    define_valid_test_case("hotp (issuer only as param)",
                           QStringLiteral("otpauth://hotp/name?secret=VALUE&counter=42&issuer=issuer"),
                           uri::QrParts::Type::Hotp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QString(), QString(), QString(), QStringLiteral("42"));
    define_valid_test_case("hotp (inconsistent issuer)",
                           QStringLiteral("otpauth://hotp/issuer:name?secret=VALUE&counter=42&issuer=other"),
                           uri::QrParts::Type::Hotp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QString(), QString(), QString(), QStringLiteral("42"));
    define_valid_test_case("hotp (empty counter)",
                           QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter="),
                           uri::QrParts::Type::Hotp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QStringLiteral("30"), QStringLiteral("6"), QStringLiteral("sha1"),
                           QString());
    define_valid_test_case("hotp (missing counter)",
                           QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1"),
                           uri::QrParts::Type::Hotp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QStringLiteral("30"), QStringLiteral("6"), QStringLiteral("sha1"),
                           QString());
    define_valid_test_case("totp (all fields, including redundant counter)",
                           QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42"),
                           uri::QrParts::Type::Totp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QStringLiteral("30"), QStringLiteral("6"), QStringLiteral("sha1"),
                           QStringLiteral("42"));
    define_valid_test_case("totp (all fields, except redundant counter)",
                           QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1"),
                           uri::QrParts::Type::Totp, QStringLiteral("name"), QStringLiteral("issuer"),
                           QStringLiteral("VALUE"), QStringLiteral("30"), QStringLiteral("6"), QStringLiteral("sha1"),
                           QString());
    define_valid_test_case("totp (minimal)",
                           QStringLiteral("otpauth://totp/name?secret=VALUE"),
                           uri::QrParts::Type::Totp, QStringLiteral("name"), QString(), QStringLiteral("VALUE"), QString(),
                           QString(), QString(), QString());
    define_valid_test_case("totp (minimal, without name)",
                           QStringLiteral("otpauth://totp?secret=VALUE"),
                           uri::QrParts::Type::Totp, QString(), QString(), QStringLiteral("VALUE"), QString(),
                           QString(), QString(), QString());
    define_valid_test_case("hotp (with padding in secret)",
                           QStringLiteral("otpauth://hotp?secret=VALUE==="),
                           uri::QrParts::Type::Hotp, QString(), QString(), QStringLiteral("VALUE==="), QString(),
                           QString(), QString(), QString());
    define_valid_test_case("totp (with padding in secret)",
                           QStringLiteral("otpauth://totp?secret=VALUE===&period=30"),
                           uri::QrParts::Type::Totp, QString(), QString(), QStringLiteral("VALUE==="),
                           QStringLiteral("30"), QString(), QString(), QString());
    define_valid_test_case("totp (Microsoft organization account)",
                           QStringLiteral("otpauth://totp/Mega%20Corporation%20A.B.%20%26%20Co.%20Kg%3Auser%40domain?secret=abcdef&issuer=Microsoft"),
                           uri::QrParts::Type::Totp, QStringLiteral("user@domain"), QStringLiteral("Mega Corporation A.B. & Co. Kg"), QStringLiteral("abcdef"),
                           QString(), QString(), QString(), QString());
}

void UriParsingTest::testInvalid(void)
{
    QFETCH(QString, input);
    QVERIFY2(!uri::QrParts::parse(input), "should reject invalid input");
}

void UriParsingTest::testInvalid_data(void)
{
    QTest::addColumn<QString>("input");

    QTest::newRow("wrong scheme") << QStringLiteral("http://localhost");
    QTest::newRow("missing type") << QStringLiteral("otpauth:///issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("unsupported type") << QStringLiteral("otpauth://wrong/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("invalid param") << QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42&foo=bar");
    QTest::newRow("missing secret") << QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("empty secret") << QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("secret = bad utf8") << QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=%c0%7fALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("secret = invalid percent encoding") << QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=%VALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("counter = bad utf8") << QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=%fc%80%80%80%80%80%80");
    QTest::newRow("counter = invalid percent encoding") << QStringLiteral("otpauth://hotp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42%");
    QTest::newRow("name label = bad utf8") << QStringLiteral("otpauth://hotp/issuer:name%c1%00?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("name label = invalid percent encoding") << QStringLiteral("otpauth://hotp/issuer:n%#ame?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("issuer label = bad utf8") << QStringLiteral("otpauth://hotp/issuer%90:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("issuer label = invalid percent encoding") << QStringLiteral("otpauth://hotp/is%suer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha1&counter=42");
    QTest::newRow("issuer param = bad utf8") << QStringLiteral("otpauth://totp/issuer:name?issuer=issuer%cf&secret=VALUE&period=30&digits=%80&algorithm=sha1");
    QTest::newRow("issuer param = invalid percent encoding") << QStringLiteral("otpauth://totp/issuer:name?issuer=issu%er&secret=VALUE&period=30&digits=%S&algorithm=sha1");
    QTest::newRow("digits = bad utf8") << QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=%80&algorithm=sha1");
    QTest::newRow("digits = invalid percent encoding") << QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=%S&algorithm=sha1");
    QTest::newRow("period = bad utf8") << QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=%c0&digits=6&algorithm=sha1");
    QTest::newRow("period = invalid percent encoding") << QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=3%&digits=6&algorithm=sha1");
    QTest::newRow("algorithm = bad utf8") << QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha%ff");
    QTest::newRow("algorithm = invalid percent encoding") << QStringLiteral("otpauth://totp/issuer:name?issuer=issuer&secret=VALUE&period=30&digits=6&algorithm=sha%1");
    QTest::newRow("hotp without params") << QStringLiteral("otpauth://hotp/issuer:name");
    QTest::newRow("totp without params") << QStringLiteral("otpauth://hotp/name");
}

QTEST_APPLESS_MAIN(UriParsingTest)

#include "qr-parsing.moc"
