/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "hmac/hmac.h"

#include <QTest>
#include <QtDebug>

class HMacValidateKeySizeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidation(void);
    void testValidation_data(void);
};

static void define_test_data(void)
{
    QTest::addColumn<int>("hash");
    QTest::addColumn<int>("keySize");
    QTest::addColumn<bool>("requireSaneKeySize");
    QTest::addColumn<bool>("expected");
}

static void define_test_case(const char *testCase, QCryptographicHash::Algorithm hash, int keySize, bool requireSaneKeySize, bool expected)
{
    QTest::newRow(qPrintable(QLatin1String(testCase))) << ((int)hash) << keySize << requireSaneKeySize << expected;
}

void HMacValidateKeySizeTest::testValidation(void)
{
    QFETCH(int, hash);
    QFETCH(int, keySize);
    QFETCH(bool, requireSaneKeySize);

    QTEST(hmac::validateKeySize((QCryptographicHash::Algorithm)hash, keySize, requireSaneKeySize), "expected");
}

void HMacValidateKeySizeTest::testValidation_data(void)
{
    define_test_data();

    define_test_case("short keys for HMAC-SHA1 permitted", QCryptographicHash::Sha1, 19, false, true);
    define_test_case("exact match for HMAC-SHA1 output", QCryptographicHash::Sha1, 20, true, true);
    define_test_case("long keys for HMAC-SHA1", QCryptographicHash::Sha1, 500, true, true);
    define_test_case("short keys for HMAC-SHA1 disallowed", QCryptographicHash::Sha1, 19, true, false);
    define_test_case("invalid key size: -1", QCryptographicHash::Sha1, -1, false, false);
    define_test_case("invalid algorithm: -1", (QCryptographicHash::Algorithm)-1, 500, true, false);
}

QTEST_APPLESS_MAIN(HMacValidateKeySizeTest)

#include "hmac-validate-keysize.moc"
