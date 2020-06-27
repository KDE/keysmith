/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath/oath.h"

#include <QTest>
#include <QtDebug>

class TotpAlgorithmTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void rfcTestVector(void);
    void rfcTestVector_data(void);
};

void define_test_case(const QString &testCase, const QString &datetime, const QByteArray &secret, QCryptographicHash::Algorithm algorithm, const char *expected)
{
    const QDateTime dt = QDateTime::fromString(datetime, Qt::ISODate);
    QTest::newRow(qPrintable(testCase.arg(datetime).arg(QLatin1String(expected)))) << dt << ((int) algorithm) << secret << QString(QLatin1String(expected));
}

void define_sha1_test_case(const char * datetime, const char *expected)
{
    define_test_case(QLatin1String("%1 (SHA1) ... %2"), QLatin1String(datetime), QByteArray("12345678901234567890"), QCryptographicHash::Sha1, expected);
}

void define_sha256_test_case(const char * datetime, const char *expected)
{
    define_test_case(QLatin1String("%1 (SHA256) ... %2"), QLatin1String(datetime), QByteArray("12345678901234567890123456789012"), QCryptographicHash::Sha256, expected);
}

void define_sha512_test_case(const char * datetime, const char *expected)
{
    define_test_case(QLatin1String("%1 (SHA512) ... %2"), QLatin1String(datetime), QByteArray("1234567890123456789012345678901234567890123456789012345678901234"), QCryptographicHash::Sha512, expected);
}

/*
 * (Static) test vector params obtained from RFC-6238
 * https://tools.ietf.org/html/rfc6238#page-9
 */
static const uint tokenLength = 8U;
static const QDateTime epoch = QDateTime::fromMSecsSinceEpoch(0);
static const uint timeStep = 30U;

void TotpAlgorithmTest::rfcTestVector(void)
{
    QFETCH(QDateTime, now);
    QFETCH(int, hash);
    QFETCH(QByteArray, secret);

    std::optional<quint64> counter = oath::count(epoch, timeStep, [now](void) -> qint64
    {
        return now.toMSecsSinceEpoch();
    });

    QVERIFY2(counter, "should be able to count timesteps using the settings of the RFC test vector");

    std::optional<oath::Algorithm> uut = oath::Algorithm::totp((QCryptographicHash::Algorithm) hash, tokenLength);
    QVERIFY2(uut, "should be able to construct the algorithm using settings of the RFC test vector");

    QByteArray copy(secret);

    std::optional<QString> result = uut->compute(*counter, copy.data(), copy.size());
    QVERIFY2(result, "token should be computed successfully");
    QTEST(*result, "rfc-test-vector");
}

void TotpAlgorithmTest::rfcTestVector_data(void)
{
    QTest::addColumn<QDateTime>("now");
    QTest::addColumn<int>("hash");
    QTest::addColumn<QByteArray>("secret");
    QTest::addColumn<QString>("rfc-test-vector");

    define_sha1_test_case("1970-01-01T00:00:59Z", "94287082");
    define_sha256_test_case("1970-01-01T00:00:59Z", "46119246");
    define_sha512_test_case("1970-01-01T00:00:59Z", "90693936");

    define_sha1_test_case("2005-03-18T01:58:29Z", "07081804");
    define_sha256_test_case("2005-03-18T01:58:29Z", "68084774");
    define_sha512_test_case("2005-03-18T01:58:29Z", "25091201");

    define_sha1_test_case("2005-03-18T01:58:31Z", "14050471");
    define_sha256_test_case("2005-03-18T01:58:31Z", "67062674");
    define_sha512_test_case("2005-03-18T01:58:31Z", "99943326");

    define_sha1_test_case("2009-02-13T23:31:30Z", "89005924");
    define_sha256_test_case("2009-02-13T23:31:30Z", "91819424");
    define_sha512_test_case("2009-02-13T23:31:30Z", "93441116");

    define_sha1_test_case("2033-05-18T03:33:20Z", "69279037");
    define_sha256_test_case("2033-05-18T03:33:20Z", "90698825");
    define_sha512_test_case("2033-05-18T03:33:20Z", "38618901");

    define_sha1_test_case("2603-10-11T11:33:20Z", "65353130");
    define_sha256_test_case("2603-10-11T11:33:20Z", "77737706");
    define_sha512_test_case("2603-10-11T11:33:20Z", "47863826");
}

QTEST_APPLESS_MAIN(TotpAlgorithmTest)

#include "totp-algorithm.moc"
