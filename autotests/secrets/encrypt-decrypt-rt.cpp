/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "secrets/secrets.h"

#include <QTest>
#include <QtDebug>

#include <string.h>

class EncryptionDecryptionRoundTripTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEncryptionDecryptionRoundTrip(void);
    void testDecryptionOfCorruptInputs(void);
};

void EncryptionDecryptionRoundTripTest::testEncryptionDecryptionRoundTrip(void)
{
    QScopedPointer<secrets::SecureMemory> passwd(secrets::SecureMemory::allocate(13ULL));
    QVERIFY2(passwd, "password memory should be allocated");
    memcpy(passwd->data(), "Hello, world!", passwd->size());

    std::optional<secrets::KeyDerivationParameters> defaults = secrets::KeyDerivationParameters::create();
    QVERIFY2(defaults, "defaults should yield a valid key parameters object");

    QScopedPointer<secrets::SecureMasterKey> masterKey(secrets::SecureMasterKey::derive(passwd.data(), *defaults));
    QVERIFY2(masterKey, "key derivation should succeed");

    QScopedPointer<secrets::SecureMemory> payload(secrets::SecureMemory::allocate(42ULL));
    QVERIFY2(payload, "allocating the secure memory input buffer should succeed");

    memset(payload->data(), 'B', 42ULL);

    std::optional<secrets::EncryptedSecret> encrypted = masterKey->encrypt(payload.data());
    QVERIFY2(encrypted, "encryption of the payload should succeed");

    QScopedPointer<secrets::SecureMemory> decrypted(masterKey->decrypt(*encrypted));
    QVERIFY2(decrypted, "decryption should succeed");

    QCOMPARE(decrypted->size(), 42ULL);

    QByteArray copyOfDecrypted;
    copyOfDecrypted.append(reinterpret_cast<const char *>(decrypted->constData()), 42ULL);

    QByteArray expected(42, 'B');
    QCOMPARE(copyOfDecrypted, expected);
}

void EncryptionDecryptionRoundTripTest::testDecryptionOfCorruptInputs(void)
{
    QScopedPointer<secrets::SecureMemory> passwd(secrets::SecureMemory::allocate(13ULL));
    QVERIFY2(passwd, "password memory should be allocated");
    memcpy(passwd->data(), "Hello, world!", passwd->size());

    std::optional<secrets::KeyDerivationParameters> defaults = secrets::KeyDerivationParameters::create();
    QVERIFY2(defaults, "defaults should yield a valid key parameters object");

    QScopedPointer<secrets::SecureMasterKey> masterKey(secrets::SecureMasterKey::derive(passwd.data(), *defaults));
    QVERIFY2(masterKey, "key derivation should succeed");

    QScopedPointer<secrets::SecureMemory> payload(secrets::SecureMemory::allocate(42ULL));
    QVERIFY2(payload, "allocating the secure memory input buffer should succeed");

    memset(payload->data(), 'B', 42ULL);

    std::optional<secrets::EncryptedSecret> encrypted = masterKey->encrypt(payload.data());
    QVERIFY2(encrypted, "encryption of the payload should succeed");

    QByteArray brokenTag(encrypted->cryptText());
    brokenTag[0] = brokenTag[0] ^ ((char) 0xFF);

    std::optional<secrets::EncryptedSecret> fakedTag = secrets::EncryptedSecret::from(brokenTag, encrypted->nonce());
    QVERIFY2(fakedTag, "should be able to construct the 'fake' encrypted input (tag)");

    QScopedPointer<secrets::SecureMemory> decryptedUsingFakeTag(masterKey->decrypt(*fakedTag));
    QVERIFY2(!decryptedUsingFakeTag, "decryption should fail when the authentication tag has been tampered with");

    QByteArray brokenPayload(encrypted->cryptText());
    brokenPayload[brokenPayload.size() - 1] = brokenPayload[brokenPayload.size() - 1] ^ ((char) 0xFF);

    std::optional<secrets::EncryptedSecret> fakedPayload = secrets::EncryptedSecret::from(brokenPayload, encrypted->nonce());
    QVERIFY2(fakedPayload, "should be able to construct the 'fake' encrypted input (payload)");

    QScopedPointer<secrets::SecureMemory> decryptedUsingFakePayload(masterKey->decrypt(*fakedPayload));
    QVERIFY2(!decryptedUsingFakePayload, "decryption should fail when the payload has been tampered with");

    QByteArray brokenNonce(encrypted->nonce());
    brokenNonce[0] = brokenNonce[0] ^ ((char) 0xFF);

    std::optional<secrets::EncryptedSecret> fakedNonce = secrets::EncryptedSecret::from(encrypted->cryptText(), brokenNonce);
    QVERIFY2(fakedNonce, "should be able to construct the 'fake' encrypted input (nonce)");

    QScopedPointer<secrets::SecureMemory> decryptedUsingFakeNonce(masterKey->decrypt(*fakedNonce));
    QVERIFY2(!decryptedUsingFakeNonce, "decryption should fail when the nonce has been tampered with");
}

QTEST_APPLESS_MAIN(EncryptionDecryptionRoundTripTest)

#include "encrypt-decrypt-rt.moc"
