/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath/oath.h"

#include <QTest>
#include <QtDebug>

class HotpAlgorithmTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void rfcTestVector(void);
    void rfcTestVector_data(void);
};

void define_test_case(quint64 counter, bool checksum, const char *expected)
{
    static const QString testCase(QLatin1String("%1 (%2) ... %3"));

    QTest::newRow(
        qPrintable(testCase.arg(counter).arg(checksum ? QLatin1String("with checksum") : QLatin1String("without checksum")).arg(QLatin1String(expected))))
        << counter << checksum << QString(QLatin1String(expected));
}

/*
 * (Static) test vector params obtained from RFC-4226
 * https://tools.ietf.org/html/rfc4226#page-32
 */
static QByteArray secret("12345678901234567890");
static uint tokenLength = 6;

void HotpAlgorithmTest::rfcTestVector(void)
{
    QFETCH(quint64, counter);
    QFETCH(bool, checksum);

    std::optional<oath::Algorithm> uut = oath::Algorithm::hotp(std::nullopt, tokenLength, checksum);
    QVERIFY2(uut, "should be able to construct the algorithm using settings of the RFC test vector");

    QByteArray copy(secret);

    std::optional<QString> result = uut->compute(counter, copy.data(), copy.size());
    QVERIFY2(result, "token should be computed successfully");
    QTEST(*result, "rfc-test-vector");
}

void HotpAlgorithmTest::rfcTestVector_data(void)
{
    static const char *corpus[20]{"755224",
                                  "287082",
                                  "359152",
                                  "969429",
                                  "338314",
                                  "254676",
                                  "287922",
                                  "162583",
                                  "399871",
                                  "520489",

                                  /*
                                   * same as the first batch (from RFC test samples), but with manually added Luhn checksum:
                                   */
                                  "7552243",
                                  "2870822",
                                  "3591526",
                                  "9694290",
                                  "3383148",
                                  "2546760",
                                  "2879229",
                                  "1625839",
                                  "3998713",
                                  "5204896"};

    QTest::addColumn<quint64>("counter");
    QTest::addColumn<bool>("checksum");
    QTest::addColumn<QString>("rfc-test-vector");

    for (quint64 k = 0; k < 20; ++k) {
        define_test_case(k % 10ULL, k >= 10, corpus[k]);
    }
}

QTEST_APPLESS_MAIN(HotpAlgorithmTest)

#include "hotp-algorithm.moc"
