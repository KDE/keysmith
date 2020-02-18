/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_KEYS_H
#define ACCOUNTS_KEYS_H

#include <QByteArray>
#include <QObject>

#include "../secrets/secrets.h"

namespace accounts
{
    class AccountSecret : public QObject
    {
        Q_OBJECT
    Q_SIGNALS:
        void newPasswordNeeded(void);
        void existingPasswordNeeded(void);
        void passwordAvailable(void);
        void keyAvailable(void);
        void requestsCancelled(void);
    public:
        AccountSecret(const secrets::SecureRandom &random = secrets::defaultSecureRandom, QObject *parent = nullptr);
        void cancelRequests(void);
        bool requestNewPassword(void);
        bool requestExistingPassword(const QByteArray& salt, const secrets::KeyDerivationParameters &keyParams);

        bool answerExistingPassword(QString &password);
        bool answerNewPassword(QString &password, const secrets::KeyDerivationParameters &keyParams);

        secrets::SecureMasterKey * deriveKey(void);

        secrets::SecureMasterKey * key(void) const;
        bool isStillAlive(void) const;
        bool isNewPasswordRequested(void) const;
        bool isExistingPasswordRequested(void) const;
        bool isKeyAvailable(void) const;
        bool isPasswordAvailable(void) const;
    private:
        bool acceptPassword(QString &password, bool answerMatchesRequest);
    private:
        bool m_stillAlive;
        bool m_newPassword;
        bool m_passwordRequested;
        const secrets::SecureRandom m_random;
        std::optional<QByteArray> m_salt;
        QScopedPointer<secrets::SecureMasterKey> m_key;
        QScopedPointer<secrets::SecureMemory> m_password;
        std::optional<secrets::KeyDerivationParameters> m_keyParams;
    };
}

#endif
