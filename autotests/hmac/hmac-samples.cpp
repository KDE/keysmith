/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "hmac/hmac.h"

#include <QMessageAuthenticationCode>
#include <QTest>
#include <QtDebug>

class HMacSamplesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAgainstQMessageAuthenticationCode(void);
    void testAgainstQMessageAuthenticationCode_data(void);
};

static const QByteArray helloWorldKey("hello, world!");
static const QByteArray helloWorldMessage("Hello, world!");

static const QByteArray longerThanMaxBlockSizeKey(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
    "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

static void define_test_data(void)
{
    QTest::addColumn<int>("hash");
    QTest::addColumn<QByteArray>("key");
    QTest::addColumn<QByteArray>("message");
    QTest::addColumn<QByteArray>("mac");
}

static void define_test_case(const char *testCase, QCryptographicHash::Algorithm hash, const QByteArray &key, const QByteArray &message)
{
    QMessageAuthenticationCode referenceImpl(hash, key);
    referenceImpl.addData(message);
    QByteArray expected = referenceImpl.result().toHex();

    QTest::newRow(qPrintable(QLatin1String(testCase))) << ((int)hash) << key << message << expected;
}

void HMacSamplesTest::testAgainstQMessageAuthenticationCode(void)
{
    QFETCH(int, hash);
    QFETCH(QByteArray, key);
    QFETCH(QByteArray, message);

    QByteArray scratchKey(key);
    const std::optional<QByteArray> result = hmac::compute((QCryptographicHash::Algorithm)hash, scratchKey.data(), scratchKey.size(), message, false);
    QVERIFY2(result, "mac should be computed successfully");
    QTEST(result->toHex(), "mac");
}

void HMacSamplesTest::testAgainstQMessageAuthenticationCode_data(void)
{
    define_test_data();

    define_test_case("'hello, world' in HMAC-SHA1", QCryptographicHash::Sha1, helloWorldKey, helloWorldMessage); // ce9935b62371cc0cb2c31b016af3fe78a5a9c9c6
    define_test_case("long key in HMAC-SH1",
                     QCryptographicHash::Sha1,
                     longerThanMaxBlockSizeKey,
                     helloWorldMessage); // 1ed7533a4f28ab52729a4a102edae6d0b7b6a049
}

QTEST_APPLESS_MAIN(HMacSamplesTest)

#include "hmac-samples.moc"
