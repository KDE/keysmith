/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "secret.h"

#include <QScopedPointer>
#include <QtDebug>

#include <cstring>

namespace test
{
    secrets::SecureMasterKey * useDummyPassword(accounts::AccountSecret *secret)
    {
        QByteArray salt;
        salt.resize(crypto_pwhash_SALTBYTES);
        salt.fill('\x0', -1);
        QString password(QLatin1String("password"));
        return useDummyPassword(secret, password, salt);
    }

    secrets::SecureMasterKey * useDummyPassword(accounts::AccountSecret *secret, QString &password, QByteArray &salt)
    {
        if (!secret) {
            qDebug () << "No account secret provided...";
            return nullptr;
        }

        std::optional<secrets::KeyDerivationParameters> keyParams = secrets::KeyDerivationParameters::create(
            crypto_secretbox_KEYBYTES, crypto_pwhash_ALG_DEFAULT, crypto_pwhash_MEMLIMIT_MIN, crypto_pwhash_OPSLIMIT_MIN
        );
        if (!keyParams) {
            qDebug () << "Failed to construct key derivation parameters";
            return nullptr;
        }

        if (!secret->requestExistingPassword(salt, *keyParams)) {
            qDebug() << "Failed to simulate password request";
            return nullptr;
        }
        if (!secret->answerExistingPassword(password)) {
            qDebug() << "Failed to supply the password";
            return nullptr;
        }

        secrets::SecureMasterKey * k = secret->deriveKey();
        if (!k) {
            qDebug() << "Failed to derive the master key";
            return nullptr;
        }
        return k;
    }

    std::optional<secrets::EncryptedSecret> encrypt(const accounts::AccountSecret *secret, const QByteArray &tokenSecret)
    {
        QScopedPointer<secrets::SecureMemory> memory(secrets::SecureMemory::allocate((size_t) tokenSecret.size()));
        if (!memory) {
            qDebug () << "Failed to set up secure memory region for token secret";
            return std::nullopt;
        }

        std::memcpy(memory->data(), tokenSecret.constData(), memory->size());
        std::optional<secrets::EncryptedSecret> s = secret->encrypt(memory.data());
        if (!s) {
            qDebug () << "Failed to encrypt token secret";
            return std::nullopt;
        }
        return s;
    }
}
