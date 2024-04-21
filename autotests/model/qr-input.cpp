/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "model/input.h"
#include "model/qr.h"

#include <QTest>

class QrInputTest : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void testValidUri(void);
    void testValidUri_data(void);
    void testInvalidUri(void);
    void testInvalidUri_data(void);
};

static void define_valid_test_case(const char *testCase,
                                   const QString &uri,
                                   const QString &name,
                                   const QString &issuer,
                                   const QString &secret,
                                   const QString &counter,
                                   uint tokenLength,
                                   uint timeStep,
                                   model::AccountInput::TokenType type,
                                   model::AccountInput::TOTPAlgorithm algorithm)
{
    QTest::newRow(testCase) << uri << name << issuer << secret << counter << tokenLength << timeStep << type << algorithm;
}

static void define_invalid_test_case(const char *testCase, const QString &uri)
{
    QTest::newRow(testCase) << uri;
}

void QrInputTest::testValidUri(void)
{
    QFETCH(QString, uri);

    model::AccountInput target;

    auto result = model::QrParameters::parse(uri);
    QVERIFY2(result, "should be able to parse valid URIs");

    result->populate(&target);

    QTEST(target.name(), "name");
    QTEST(target.issuer(), "issuer");
    QTEST(target.secret(), "secret");
    QTEST(target.counter(), "counter");
    QTEST(target.tokenLength(), "tokenLength");
    QTEST(target.timeStep(), "timeStep");
    QTEST(target.type(), "type");
    QTEST(target.algorithm(), "algorithm");
}

void QrInputTest::testValidUri_data(void)
{
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("issuer");
    QTest::addColumn<QString>("secret");
    QTest::addColumn<QString>("counter");
    QTest::addColumn<uint>("tokenLength");
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<model::AccountInput::TokenType>("type");
    QTest::addColumn<model::AccountInput::TOTPAlgorithm>("algorithm");

    define_valid_test_case("hotp (all fields set)",
                           QStringLiteral("otpauth://hotp/issuer:valid?secret=VALUE&digits=8&period=60&issuer=issuer&counter=42&algorithm=sha512"),
                           QStringLiteral("valid"),
                           QStringLiteral("issuer"),
                           QStringLiteral("VALUE==="),
                           QStringLiteral("42"),
                           8U,
                           60U,
                           model::AccountInput::TokenType::Hotp,
                           model::AccountInput::TOTPAlgorithm::Sha512);
}

void QrInputTest::testInvalidUri(void)
{
    QFETCH(QString, uri);
    QVERIFY2(!model::QrParameters::parse(uri), "should reject invalid URIs");
}

void QrInputTest::testInvalidUri_data(void)
{
    QTest::addColumn<QString>("uri");

    define_invalid_test_case("token length not a number", QStringLiteral("otpauth://totp/invalid?secret=VALUE&digits=nan"));
    define_invalid_test_case("token length too small", QStringLiteral("otpauth://totp/invalid?secret=VALUE&digits=2"));
    define_invalid_test_case("token length too large", QStringLiteral("otpauth://totp/invalid?secret=VALUE&digits=22"));

    define_invalid_test_case("time step not a number", QStringLiteral("otpauth://totp/invalid?secret=VALUE&period=nan"));
    define_invalid_test_case("time step too small", QStringLiteral("otpauth://totp/invalid?secret=VALUE&period=0"));
    define_invalid_test_case("time step too large", QStringLiteral("otpauth://totp/invalid?secret=VALUE&period=999999999999"));

    define_invalid_test_case("invalid base32 secret", QStringLiteral("otpauth://totp/invalid?secret=19"));
    define_invalid_test_case("empty secret", QStringLiteral("otpauth://totp/invalid?secret="));

    define_invalid_test_case("counter not a number", QStringLiteral("otpauth://hotp/invalid?secret=VALUE&counter=nan"));

    define_invalid_test_case("invalid algorithm ", QStringLiteral("otpauth://totp/invalid?secret=VALUE&algorithm=foo"));

    define_invalid_test_case("name contains non-printable characters", QStringLiteral("otpauth://totp/inv%00alid?secret=VALUE"));

    define_invalid_test_case("issuer contains non-printable characters", QStringLiteral("otpauth://totp/iss%00:invalid?secret=VALUE"));
    define_invalid_test_case("issuer parameter contains non-printable characters", QStringLiteral("otpauth://totp/invalid?secret=VALUE&issuer=iss%03"));
}

QTEST_APPLESS_MAIN(QrInputTest)

#include "qr-input.moc"
