/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "keys.h"

#include "../logging_p.h"

#include <cstring>

KEYSMITH_LOGGER(logger, ".accounts.keys")

namespace accounts
{
AccountSecret::AccountSecret(const secrets::SecureRandom &random, QObject *parent)
    : QObject(parent)
    , m_stillAlive(true)
    , m_newPassword(false)
    , m_passwordRequested(false)
    , m_hackWithoutChallenge(false)
    , // HACK: disables challenge verification, remove at some point!
    m_random(random)
    , m_salt(std::nullopt)
    , m_challenge(std::nullopt)
    , m_key(nullptr)
    , m_password(nullptr)
    , m_keyParams(std::nullopt)
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

bool AccountSecret::requestExistingPassword(const secrets::EncryptedSecret &challenge,
                                            const QByteArray &salt,
                                            const secrets::KeyDerivationParameters &keyParams)
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
    m_challenge.emplace(challenge);
    Q_EMIT existingPasswordNeeded();
    return true;
}

// HACK: disables challenge verification, remove at some point!
bool AccountSecret::requestExistingPassword(const QByteArray &salt, const secrets::KeyDerivationParameters &keyParams)
{
    qCWarning(logger) << "HACK: running a 'silent' migration for legacy account setups without password challenge";

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
    m_hackWithoutChallenge = true;
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

    if (m_key || (m_password && !m_challenge)) {
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
    m_password.reset(secrets::SecureMemory::allocate((size_t)passwordBytes.size()));

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
    return !m_password.isNull();
}

bool AccountSecret::answerExistingPassword(QString &password)
{
    // HACK: disables challenge verification, remove at some point!
    bool challengeOk = (m_challenge || m_hackWithoutChallenge);
    bool result = acceptPassword(password, m_keyParams && m_salt && challengeOk);
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

    bool result = acceptPassword(password, !m_keyParams && !m_salt && !m_challenge);
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

bool AccountSecret::isChallengeAvailable(void) const
{
    return m_stillAlive && m_challenge;
}

secrets::SecureMasterKey *AccountSecret::deriveKey(void)
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

    secrets::SecureMasterKey *derived = m_salt ? secrets::SecureMasterKey::derive(m_password.data(), *m_keyParams, *m_salt, m_random)
                                               : secrets::SecureMasterKey::derive(m_password.data(), *m_keyParams, m_random);

    if (!derived) {
        qCDebug(logger) << "Failed to derive encryption/decryption key for account secrets";
        m_password.reset(nullptr);
        Q_EMIT keyFailed();
        return nullptr;
    }

    if (m_challenge) {
        QScopedPointer<secrets::SecureMemory> result(derived->decrypt(*m_challenge));
        if (!result) {
            qCDebug(logger) << "Failed to derive encryption/decryption key for account secrets: challenge failed";
            m_password.reset(nullptr);
            delete derived;
            Q_EMIT keyFailed();
            return nullptr;
        }

        bool sizeMismatch = result->size() != m_password->size();
        const unsigned char *const other = sizeMismatch ? m_password->constData() : result->constData();
        if (std::memcmp(m_password->constData(), other, m_password->size()) != 0 || sizeMismatch) {
            qCDebug(logger) << "Failed to derive encryption/decryption key for account secrets: challenge failed";
            m_password.reset(nullptr);
            delete derived;
            Q_EMIT keyFailed();
            return nullptr;
        }
    } else {
        std::optional<secrets::EncryptedSecret> challenge = derived->encrypt(m_password.data());
        if (!challenge) {
            qCDebug(logger) << "Failed to derive encryption/decryption key for account secrets: unable to generate challenge";
            m_password.reset(nullptr);
            delete derived;
            Q_EMIT keyFailed();
            return nullptr;
        }
        m_challenge.emplace(*challenge);
    }

    qCDebug(logger) << "Successfully derived encryption/decryption key for account secrets";
    m_key.reset(derived);
    m_salt.emplace(m_key->salt());
    m_password.reset(nullptr);
    Q_EMIT keyAvailable();
    return m_key.data();
}

secrets::SecureMasterKey *AccountSecret::key(void) const
{
    return isKeyAvailable() ? m_key.data() : nullptr;
}

std::optional<secrets::EncryptedSecret> AccountSecret::challenge(void) const
{
    return isChallengeAvailable() ? m_challenge : std::nullopt;
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

secrets::SecureMemory *AccountSecret::decrypt(const secrets::EncryptedSecret &secret) const
{
    secrets::SecureMasterKey *k = key();
    if (!k) {
        qCDebug(logger) << "Unable to decrypt secret: decryption key not available";
        return nullptr;
    }

    return k->decrypt(secret);
}
}

#include "moc_keys.cpp"
