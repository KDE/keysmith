/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "password.h"

#include "../secrets/secrets.h"
#include "../logging_p.h"

KEYSMITH_LOGGER(logger, ".model.password")

namespace model
{
    PasswordRequest::PasswordRequest(accounts::AccountSecret *secret, QObject *parent) :
        QObject(parent), m_secret(secret), m_previous(false), m_haveKey(false), m_havePassword(false)
    {
        QObject::connect(m_secret, &accounts::AccountSecret::existingPasswordNeeded, this, &PasswordRequest::setPreviouslyDefined);
        QObject::connect(m_secret, &accounts::AccountSecret::passwordAvailable, this, &PasswordRequest::setPasswordAvailable);
        QObject::connect(m_secret, &accounts::AccountSecret::keyAvailable, this, &PasswordRequest::setKeyAvailable);
        m_previous = secret->isExistingPasswordRequested();
        m_haveKey = secret->isKeyAvailable();
        m_havePassword = secret->isPasswordAvailable();
    }

    bool PasswordRequest::previouslyDefined(void) const
    {
        return m_previous;
    }

    bool PasswordRequest::keyAvailable(void) const
    {
        return m_haveKey;
    }

    bool PasswordRequest::passwordProvided(void) const
    {
        return m_havePassword;
    }

    bool PasswordRequest::provideBothPasswords(QString password, QString other)
    {
        if (password != other || password.isEmpty()) {
            qCDebug(logger) << "Not applying new password(s): passwords must match and must not be empty";
            return false;
        }

        if (m_previous) {
            qCDebug(logger) << "Ignoring new password(s): function should not be used to unlock existing account secrets";
            return false;
        }

        std::optional<secrets::KeyDerivationParameters> params = secrets::KeyDerivationParameters::create();
        if (!params) {
            qCDebug(logger) << "Unable apply new password(s): failed to create default key derivation parameters";
            return false;
        }

        if (m_secret->answerNewPassword(password, *params)) {
            other.fill(QLatin1Char('*'), -1);
            return true;
        }

        qCDebug(logger) << "Failed to apply new password(s)";
        return false;
    }

    bool PasswordRequest::providePassword(QString password)
    {
        if (password.isEmpty()) {
            qCDebug(logger) << "Not applying password: passwords must not be empty";
            return false;
        }

        if (!m_previous) {
            qCDebug(logger) << "Ignoring password: function should not be used to set up new account secrets";
            return false;
        }

        if (m_secret->answerExistingPassword(password)) {
            password.fill(QLatin1Char('*'), -1);
            return true;
        }

        qCDebug(logger) << "Failed to apply password for existing account secrets";
        return false;
    }

    void PasswordRequest::setKeyAvailable(void)
    {
        if (!m_haveKey) {
            m_haveKey = true;
            Q_EMIT derivedKey();
        } else {
            qCDebug(logger) << "Ignored signal: already marked key as available";
        }
    }

    void PasswordRequest::setPasswordAvailable(void)
    {
        if (!m_havePassword) {
            m_havePassword = true;
            Q_EMIT passwordAccepted();
        } else {
            qCDebug(logger) << "Ignored signal: already marked password as available";
        }
    }

    void PasswordRequest::setPreviouslyDefined(void)
    {
        if (!m_previous) {
            m_previous = true;
            Q_EMIT passwordExists();
        } else {
            qCDebug(logger) << "Ignored signal: already marked password for existing secrets";
        }
    }
}
