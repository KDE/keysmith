/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef SECRETS_H
#define SECRETS_H

#include "sodium_cpp.h"
#include "../hmac/hmac.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QString>

#include <functional>
#include <optional>

namespace secrets
{
    class EncryptedSecret
    {
    public:
        static std::optional<EncryptedSecret> from(const QByteArray& cryptText, const QByteArray& nonce);
        const QByteArray& cryptText(void) const;
        const QByteArray& nonce(void) const;
        unsigned long long messageLength(void) const;
    private:
        EncryptedSecret(const QByteArray &taggedCryptText, const QByteArray &nonce);
    private:
        const QByteArray m_taggedCryptText;
        const QByteArray m_nonce;
    };

    class KeyDerivationParameters
    {
    public:
        static std::optional<KeyDerivationParameters> create(unsigned long long keyLength = crypto_secretbox_KEYBYTES,
                                                      int algorithm = crypto_pwhash_ALG_DEFAULT,
                                                      size_t memoryCost = crypto_pwhash_MEMLIMIT_MODERATE,
                                                      unsigned long long cpuCost = crypto_pwhash_OPSLIMIT_MODERATE);
        int algorithm(void) const;
        unsigned long long keyLength(void) const;
        size_t memoryCost(void) const;
        unsigned long long cpuCost(void) const;
    private:
        KeyDerivationParameters(int algorithm, unsigned long long keyLength, size_t memoryCost, unsigned long long cpuCost);
    private:
        const int m_algorithm;
        const unsigned long long m_keyLength;
        const size_t m_memoryCost;
        const unsigned long long m_cpuCost;
    };

    class SecureMemory
    {
    public:
        static SecureMemory * allocate(size_t size);
        virtual ~SecureMemory();
        size_t size(void) const;
        const unsigned char * constData(void) const;
        unsigned char * data(void);
    private:
        SecureMemory(void * memory, size_t size);
        Q_DISABLE_COPY(SecureMemory)
    private:
        unsigned char * const m_secureMemory;
        const size_t m_size;
    };

    using SecureRandom = std::function<bool(void *, size_t)>;
    bool defaultSecureRandom(void *data, size_t size);

    class SecureMasterKey
    {
    public:
        static bool validate(const SecureMemory *secret);
        static bool validate(const KeyDerivationParameters &params);
        static bool validate(const QByteArray &salt);
        static SecureMasterKey * derive(const SecureMemory *password, const KeyDerivationParameters &params, const SecureRandom &random = defaultSecureRandom);
        static SecureMasterKey * derive(const SecureMemory *password, const KeyDerivationParameters &params, const QByteArray &salt, const SecureRandom &random = defaultSecureRandom);
    public:
        const KeyDerivationParameters params(void) const;
        const QByteArray salt(void) const;
        virtual ~SecureMasterKey();
        std::optional<EncryptedSecret> encrypt(const SecureMemory *secret) const;
        SecureMemory * decrypt(const EncryptedSecret &secret) const;
    private:
        SecureMasterKey(unsigned char *keyMemory, const KeyDerivationParameters &params, const QByteArray &salt, const SecureRandom &random);
        Q_DISABLE_COPY(SecureMasterKey)
    private:
        unsigned char * const m_secureKeyMemory;
        const KeyDerivationParameters m_params;
        const QByteArray m_salt;
        const SecureRandom m_secureRandom;
    };

    SecureMemory * decodeBase32(const QString &encoded, int from = 0, int until = -1);
}

#endif
