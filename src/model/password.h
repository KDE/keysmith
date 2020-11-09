/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef MODEL_PASSWORD_H
#define MODEL_PASSWORD_H

#include "../account/keys.h"

namespace model
{
    class PasswordRequest: public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool firstRun READ firstRun NOTIFY passwordRequestChanged)
        Q_PROPERTY(bool previouslyDefined READ previouslyDefined NOTIFY passwordRequestChanged)
        Q_PROPERTY(bool keyAvailable READ keyAvailable NOTIFY derivedKey)
        Q_PROPERTY(bool passwordProvided READ passwordProvided NOTIFY passwordAccepted)
    public:
        explicit PasswordRequest(accounts::AccountSecret *secret, QObject *parent = nullptr);
        bool firstRun(void) const;
        bool previouslyDefined(void) const;
        bool keyAvailable(void) const;
        bool passwordProvided(void) const;
    public:
        Q_INVOKABLE bool providePassword(QString password);
        Q_INVOKABLE bool provideBothPasswords(QString password, QString other);
    Q_SIGNALS:
        void passwordRequestChanged(void);
        void passwordExists(void);
        void newPasswordNeeded(void);
        void passwordAccepted(void);
        void derivedKey(void);
    private Q_SLOTS:
        void setKeyAvailable(void);
        void setPasswordAvailable(void);
        void setPreviouslyDefined(void);
        void setNewPasswordNeeded(void);
    private:
        accounts::AccountSecret * m_secret;
    private:
        bool m_firstRun;
        bool m_previous;
        bool m_haveKey;
        bool m_havePassword;
    };
}

#endif
