/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "keys.h"

#include "../logging_p.h"

#include <cstring>

KEYSMITH_LOGGER(logger, ".accounts.keys")

namespace accounts
{
    AccountSecret::AccountSecret(const secrets::SecureRandom &random, QObject *parent) :
        QObject(parent), m_stillAlive(true), m_newPassword(false), m_passwordRequested(false), m_random(random), m_salt(std::nullopt), m_key(nullptr), m_password(nullptr), m_keyParams(std::nullopt)
    {
    }

    void AccountSecret::cancelRequests(void)
    {
        if (!m_stillAlive) {
            qCDebug(logger) << "Ignoring cancellation request: account secret is marked for death";
            return;
        }

        m_stillAlive = false;
        Q_EMIT requestsCancelled();
    }

    bool AccountSecret::requestNewPassword(void)
    {
        if (!m_stillAlive) {
            qCDebug(logger) << "Ignoring request for 'new' password: account secret is marked for death";
            return false;
        }

        if (m_passwordRequested) {
            qCDebug(logger) << "Ignoring request for 'new' password: conflicting or duplicate request";
            return false;
        }

        qCDebug(logger) << "Emitting request for 'new' password";
        m_passwordRequested = true;
        m_newPassword = true;
        Q_EMIT newPasswordNeeded();
        return true;
    }

    bool AccountSecret::requestExistingPassword(const QByteArray& salt, const secrets::KeyDerivationParameters &keyParams)
    {
        if (!m_stillAlive) {
            qCDebug(logger) << "Ignoring request for 'existing' password: account secret is marked for death";
            return false;
        }

        if (m_passwordRequested) {
            qCDebug(logger) << "Ignoring request for 'existing' password: conflicting or duplicate request";
            return false;
        }

        if (!secrets::SecureMasterKey::validate(keyParams)) {
            qCDebug(logger) << "Unable to request 'existing' password: invalid key derivation parameters";
            return false;
        }

        if (!secrets::SecureMasterKey::validate(salt)) {
            qCDebug(logger) << "Unable to request 'existing' password: invalid salt";
            return false;
        }

        qCDebug(logger) << "Emitting request for 'existing' password";
        m_passwordRequested = true;
        m_newPassword = false;
        m_keyParams.emplace(keyParams);
        m_salt.emplace(salt);
        Q_EMIT existingPasswordNeeded();
        return true;
    }

    bool AccountSecret::acceptPassword(QString &password, bool answerMatchesRequest)
    {
        QByteArray passwordBytes;
        if (!m_stillAlive) {
            qCDebug(logger) << "Ignoring password: account secret is marked for death";
            password.fill(QLatin1Char('*'), -1);
            return false;
        }

        if (!m_passwordRequested) {
            qCDebug(logger) << "Ignoring password: was not requested";
            password.fill(QLatin1Char('*'), -1);
            return false;
        }

        if (m_key || m_password) {
            qCDebug(logger) << "Ignoring password: duplicate/conflicting password";
            password.fill(QLatin1Char('*'), -1);
            return false;
        }

        if (!answerMatchesRequest) {
            qCDebug(logger) << "Ignoring password: wrong answer function used for the request";
            password.fill(QLatin1Char('*'), -1);
            return false;
        }

        /*
         * This is still unfortunate: no idea how many (partial) copies toUtf8() makes.
         * I.e. no idea how many (partial) copies of the secret wind up floating around in memory.
         */
        passwordBytes = password.toUtf8();
        m_password.reset(secrets::SecureMemory::allocate((size_t) passwordBytes.size()));

        if (m_password) {
            qCDebug(logger) << "Accepted password for account secrets";
            std::memcpy(m_password->data(), passwordBytes.constData(), m_password->size());
        } else {
            qCDebug(logger) << "Failed to accept password for account secrets";
        }

        /*
         * Try and overwrite known copies of the password/secret (these are redundant now...)
         */
        passwordBytes.fill('\0', -1);
        password.fill(QLatin1Char('*'), -1);
        return m_password;
    }

    bool AccountSecret::answerExistingPassword(QString &password)
    {
        bool result = acceptPassword(password, m_keyParams && m_salt);
        if (result) {
            Q_EMIT passwordAvailable();
        }
        return result;
    }

    bool AccountSecret::answerNewPassword(QString &password, const secrets::KeyDerivationParameters &keyParams)
    {
        if (!secrets::SecureMasterKey::validate(keyParams)) {
            qCDebug(logger) << "Unable to accept 'existing' password: invalid key derivation parameters";
            password.fill(QLatin1Char('*'), -1);
            return false;
        }

        bool result = acceptPassword(password, !m_keyParams && !m_salt);
        if (result) {
            m_keyParams.emplace(keyParams);
            Q_EMIT passwordAvailable();
        }
        return result;
    }

    bool AccountSecret::isStillAlive(void) const
    {
        return m_stillAlive;
    }

    bool AccountSecret::isNewPasswordRequested(void) const
    {
        return m_passwordRequested && m_newPassword;
    }

    bool AccountSecret::isExistingPasswordRequested(void) const
    {
        return m_passwordRequested && !m_newPassword;
    }

    bool AccountSecret::isKeyAvailable(void) const
    {
        return m_stillAlive && m_key;
    }

    bool AccountSecret::isPasswordAvailable(void) const
    {
        return m_stillAlive && m_password;
    }

    secrets::SecureMasterKey * AccountSecret::deriveKey(void)
    {
        if (!m_stillAlive) {
            qCDebug(logger) << "Ignoring request to derive encryption/decryption key: account secret is marked for death";
            m_password.reset(nullptr);
            return nullptr;
        }

        if (m_key) {
            qCDebug(logger) << "Ignoring request to derive encryption/decryption key: duplicate request";
            m_password.reset(nullptr);
            return nullptr;
        }

        if (!m_passwordRequested || !m_keyParams || !m_password) {
            qCDebug(logger) << "Ignoring request to derive encryption/decryption key: passwor or key derivation parameters not available";
            m_password.reset(nullptr);
            return nullptr;
        }

        m_key.reset(m_salt
            ? secrets::SecureMasterKey::derive(m_password.data(), *m_keyParams, *m_salt, m_random)
            : secrets::SecureMasterKey::derive(m_password.data(), *m_keyParams, m_random)
        );

        if (!m_key) {
            qCDebug(logger) << "Failed to derive encryption/decryption key for account secrets";
            m_password.reset(nullptr);
            return nullptr;
        }

        qCDebug(logger) << "Successfully derived encryption/decryption key for account secrets";
        m_salt.emplace(m_key->salt());
        m_password.reset(nullptr);
        Q_EMIT keyAvailable();
        return m_key.data();
    }

    secrets::SecureMasterKey * AccountSecret::key(void) const
    {
        return m_stillAlive && m_key ? m_key.data() : nullptr;
    }

    std::optional<secrets::EncryptedSecret> AccountSecret::encrypt(const secrets::SecureMemory *secret) const
    {
        secrets::SecureMasterKey *k = key();
        if (!k) {
            qCDebug(logger) << "Unable to encrypt secret: encryption key not available";
            return std::nullopt;
        }

        return k->encrypt(secret);
    }

    secrets::SecureMemory * AccountSecret::decrypt(const secrets::EncryptedSecret &secret) const
    {
        secrets::SecureMasterKey *k = key();
        if (!k) {
            qCDebug(logger) << "Unable to decrypt secret: decryption key not available";
            return  nullptr;
        }

        return k->decrypt(secret);
    }
}
