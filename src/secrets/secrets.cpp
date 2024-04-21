/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "secrets.h"

#include "../base32/base32.h"
#include "../logging_p.h"

#include <QCryptographicHash>

#include <limits>

KEYSMITH_LOGGER(logger, ".secrets")

namespace secrets
{

std::optional<EncryptedSecret> EncryptedSecret::from(const QByteArray &cryptText, const QByteArray &nonce)
{
    if (cryptText.size() < 0 || ((uint)cryptText.size()) <= crypto_secretbox_MACBYTES) {
        qCDebug(logger) << "Invalid ciphertext: too short, expected at least:" << crypto_secretbox_MACBYTES << "bytes but got:" << cryptText.size();
        return std::nullopt;
    }

    if (nonce.size() != crypto_secretbox_NONCEBYTES) {
        qCDebug(logger) << "Invalid nonce: expected exactly:" << crypto_secretbox_NONCEBYTES << "but got:" << nonce.size();
        return std::nullopt;
    }

    return std::optional<EncryptedSecret>({cryptText, nonce});
}

unsigned long long EncryptedSecret::messageLength(void) const
{
    return m_taggedCryptText.size() - crypto_secretbox_MACBYTES;
}

const QByteArray &EncryptedSecret::cryptText(void) const
{
    return m_taggedCryptText;
}

const QByteArray &EncryptedSecret::nonce(void) const
{
    return m_nonce;
}

EncryptedSecret::EncryptedSecret(const QByteArray &taggedCryptText, const QByteArray &nonce)
    : m_taggedCryptText(taggedCryptText)
    , m_nonce(nonce)
{
}

std::optional<KeyDerivationParameters>
KeyDerivationParameters::create(unsigned long long keyLength, int algorithm, size_t memoryCost, unsigned long long cpuCost)
{
    switch (algorithm) {
    case crypto_pwhash_ALG_ARGON2I13:
    case crypto_pwhash_ALG_ARGON2ID13:
        break;
    default:
        qCDebug(logger) << "Unable to construct key derivation parameters: unkown algorithm:" << algorithm;
        return std::nullopt;
    }

    if (keyLength < crypto_pwhash_BYTES_MIN || keyLength > crypto_pwhash_BYTES_MAX || cpuCost < crypto_pwhash_OPSLIMIT_MIN
        || cpuCost > crypto_pwhash_OPSLIMIT_MAX || memoryCost < crypto_pwhash_MEMLIMIT_MIN || memoryCost > crypto_pwhash_MEMLIMIT_MAX) {
        return std::nullopt;
    }

    return std::optional<KeyDerivationParameters>({algorithm, keyLength, memoryCost, cpuCost});
}

int KeyDerivationParameters::algorithm(void) const
{
    return m_algorithm;
}

unsigned long long KeyDerivationParameters::keyLength(void) const
{
    return m_keyLength;
}

size_t KeyDerivationParameters::memoryCost(void) const
{
    return m_memoryCost;
}

unsigned long long KeyDerivationParameters::cpuCost(void) const
{
    return m_cpuCost;
}

KeyDerivationParameters::KeyDerivationParameters(int algorithm, unsigned long long keyLength, size_t memoryCost, unsigned long long cpuCost)
    : m_algorithm(algorithm)
    , m_keyLength(keyLength)
    , m_memoryCost(memoryCost)
    , m_cpuCost(cpuCost)
{
}

SecureMemory *SecureMemory::allocate(size_t size)
{
    if (sodium_init() < 0) {
        qCDebug(logger) << "Unable to allocate secure memory region: libsodium failed to initialise";
        return nullptr;
    }

    if (size == 0ULL) {
        qCDebug(logger) << "Rejecting attempt to allocate empty secure memory region";
        return nullptr;
    }

    void *memory = sodium_malloc(size);
    if (memory) {
        return new SecureMemory(memory, size);
    }

    qCDebug(logger) << "Failed to allocate secure memory region";
    return nullptr;
}

size_t SecureMemory::size(void) const
{
    return m_size;
}

const unsigned char *SecureMemory::constData(void) const
{
    return m_secureMemory;
}

unsigned char *SecureMemory::data(void)
{
    return m_secureMemory;
}

SecureMemory::~SecureMemory()
{
    sodium_free(m_secureMemory);
}

SecureMemory::SecureMemory(void *memory, size_t size)
    : m_secureMemory(reinterpret_cast<unsigned char *>(memory))
    , m_size(size)
{
}

bool SecureMasterKey::validate(const SecureMemory *secret)
{
    static const int max = std::numeric_limits<int>::max() - crypto_secretbox_MACBYTES;
    return max > 0 && secret->size() > 0ULL && secret->size() <= max;
}

bool SecureMasterKey::validate(const KeyDerivationParameters &params)
{
    return params.keyLength() == crypto_secretbox_KEYBYTES;
}

bool SecureMasterKey::validate(const QByteArray &salt)
{
    return salt.size() == crypto_pwhash_SALTBYTES;
}

SecureMasterKey *SecureMasterKey::derive(const SecureMemory *password, const KeyDerivationParameters &params, const SecureRandom &random)
{
    QByteArray salt;

    if (!password) {
        qCDebug(logger) << "Unable to derive a key: no password given";
        return nullptr;
    }
    if (sodium_init() < 0) {
        qCDebug(logger) << "Unable to derive a key: libsodium failed to initialise";
        return nullptr;
    }

    salt.reserve(crypto_pwhash_SALTBYTES);
    salt.resize(crypto_pwhash_SALTBYTES);

    if (!random(salt.data(), crypto_pwhash_SALTBYTES)) {
        qCDebug(logger) << "Unable to derive a key: failed to generate the random nonce";
        return nullptr;
    }

    return derive(password, params, salt, random);
}

SecureMasterKey *
SecureMasterKey::derive(const SecureMemory *password, const KeyDerivationParameters &params, const QByteArray &salt, const SecureRandom &random)
{
    if (!password) {
        qCDebug(logger) << "Unable to derive a key: no password given";
        return nullptr;
    }
    if (sodium_init() < 0) {
        qCDebug(logger) << "Unable to derive a key: libsodium failed to initialise";
        return nullptr;
    }
    if (!validate(params)) {
        qCDebug(logger) << "Unable to derive a key: invalid key derivation parameters";
        return nullptr;
    }
    if (!validate(salt)) {
        qCDebug(logger) << "Unable to derive a key: invalid salt";
        return nullptr;
    }

    void *memory = sodium_malloc((size_t)params.keyLength());
    if (!memory) {
        qCDebug(logger) << "Unable to derive a key: failed to allocate secure memory region";
        return nullptr;
    }

    unsigned char *derived = reinterpret_cast<unsigned char *>(memory);
    const unsigned char *saltData = reinterpret_cast<const unsigned char *>(salt.constData());
    const char *passwordData = reinterpret_cast<const char *>(password->constData());
    if (crypto_pwhash(derived, params.keyLength(), passwordData, password->size(), saltData, params.cpuCost(), params.memoryCost(), params.algorithm()) == 0) {
        return new SecureMasterKey(derived, params, salt, random);
    }

    qCDebug(logger) << "Failed to derive a key: password hashing failed";
    sodium_free(memory);
    return nullptr;
}

const KeyDerivationParameters SecureMasterKey::params(void) const
{
    return m_params;
}

const QByteArray SecureMasterKey::salt(void) const
{
    return m_salt;
}

std::optional<EncryptedSecret> SecureMasterKey::encrypt(const SecureMemory *secret) const
{
    QByteArray cryptText;
    QByteArray nonce;

    if (!validate(secret)) {
        qCDebug(logger) << "Unable to encrypt secret: invalid input";
        return std::nullopt;
    }

    cryptText.reserve(((int)secret->size()) + crypto_secretbox_MACBYTES);
    cryptText.resize(((int)secret->size()) + crypto_secretbox_MACBYTES);
    nonce.reserve(crypto_secretbox_NONCEBYTES);
    nonce.resize(crypto_secretbox_NONCEBYTES);

    if (!m_secureRandom(nonce.data(), crypto_secretbox_NONCEBYTES)) {
        qCDebug(logger) << "Unable to encrypt secret: failed to generate the random nonce";
        return std::nullopt;
    }

    unsigned char *encrypted = reinterpret_cast<unsigned char *>(cryptText.data());
    const unsigned char *nonceData = reinterpret_cast<const unsigned char *>(nonce.constData());
    if (crypto_secretbox_easy(encrypted, secret->constData(), (unsigned long long)secret->size(), nonceData, m_secureKeyMemory) == 0) {
        return EncryptedSecret::from(cryptText, nonce);
    }

    qCDebug(logger) << "Failed to encrypt secret";
    return std::nullopt;
}

SecureMemory *SecureMasterKey::decrypt(const EncryptedSecret &secret) const
{
    SecureMemory *result = SecureMemory::allocate(secret.messageLength());
    if (!result) {
        qCDebug(logger) << "Unable to decrypt secret: failed to allocate secure memory region";
        return nullptr;
    }

    unsigned char *decrypted = reinterpret_cast<unsigned char *>(result->data());
    const unsigned char *cryptText = reinterpret_cast<const unsigned char *>(secret.cryptText().constData());
    const unsigned char *nonce = reinterpret_cast<const unsigned char *>(secret.nonce().constData());
    if (crypto_secretbox_open_easy(decrypted, cryptText, (unsigned long long)secret.cryptText().size(), nonce, m_secureKeyMemory) == 0) {
        return result;
    }

    qCDebug(logger) << "Failed to decrypt secret";
    delete result;
    return nullptr;
}

SecureMasterKey::~SecureMasterKey()
{
    sodium_free(m_secureKeyMemory);
}

SecureMasterKey::SecureMasterKey(unsigned char *keyMemory, const KeyDerivationParameters &params, const QByteArray &salt, const SecureRandom &random)
    : m_secureKeyMemory(keyMemory)
    , m_params(params)
    , m_salt(salt)
    , m_secureRandom(random)
{
}

bool defaultSecureRandom(void *data, size_t size)
{
    if (sodium_init() < 0) {
        qCDebug(logger) << "Unable to fill buffer with random bytes: libsodium failed to initialise";
        return false;
    }

    randombytes_buf(data, size);
    return true;
}

SecureMemory *decodeBase32(const QString &encoded, int from, int until)
{
    std::optional<size_t> size = base32::validate(encoded, from, until);

    if (!size) {
        return nullptr;
    }

    SecureMemory *result = SecureMemory::allocate(*size);
    if (!result) {
        qCDebug(logger) << "Unable to decoded base32 secrets: failed to allocate secure memory";
        return nullptr;
    }

    std::optional<size_t> decoded = base32::decode(encoded, reinterpret_cast<char *>(result->data()), result->size(), from, until);
    if (decoded && *decoded == *size) {
        return result;
    }

    Q_ASSERT_X(decoded, Q_FUNC_INFO, "invalid base32 should have been caught by prior validation");
    Q_ASSERT_X(*decoded == *size, Q_FUNC_INFO, "the entire base32 input should have been decoded");
    delete result;
    return nullptr;
}
}
