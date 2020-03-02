/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "secrets/secrets.h"

#include "test-utils/random.h"

#include <QTest>
#include <QtDebug>

class KeyDerivationTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRecovery(void);
};

void KeyDerivationTest::testRecovery(void)
{
    QScopedPointer<secrets::SecureMemory> passwd(secrets::SecureMemory::allocate(13ULL));
    QVERIFY2(passwd, "password memory should be allocated");
    memcpy(passwd->data(), "Hello, world!", passwd->size());

    std::optional<secrets::KeyDerivationParameters> defaults = secrets::KeyDerivationParameters::create();
    QVERIFY2(defaults, "defaults should yield a valid key parameters object");

    QScopedPointer<secrets::SecureMasterKey> origMasterKey(secrets::SecureMasterKey::derive(passwd.data(), *defaults, &test::fakeRandom));
    QVERIFY2(origMasterKey, "key derivation should succeed");

    QByteArray expectedSalt(crypto_pwhash_SALTBYTES, 'A');
    QCOMPARE(origMasterKey->salt(), expectedSalt);

    QScopedPointer<secrets::SecureMasterKey> copyKey(secrets::SecureMasterKey::derive(passwd.data(), *defaults, expectedSalt, &test::fakeRandom));
    QVERIFY2(copyKey, "recovering/re-deriving a copy of the master key should succeed");

    QScopedPointer<secrets::SecureMemory> payload(secrets::SecureMemory::allocate(42ULL));
    QVERIFY2(payload, "allocating the secure memory input buffer should succeed");

    memset(payload->data(), 'B', 42ULL);

    std::optional<secrets::EncryptedSecret> fromOrigKey = origMasterKey->encrypt(payload.data());
    QVERIFY2(fromOrigKey, "encryption of the payload should succeed with the original master key");

    std::optional<secrets::EncryptedSecret> fromCopyKey = copyKey->encrypt(payload.data());
    QVERIFY2(fromCopyKey, "encryption of the payload should also succeed with the recovered copy of the master key");

    QCOMPARE(fromOrigKey->cryptText(), fromCopyKey->cryptText());
    QCOMPARE(fromOrigKey->nonce(), fromCopyKey->nonce());
}

QTEST_APPLESS_MAIN(KeyDerivationTest)

#include "key-derivation.moc"
