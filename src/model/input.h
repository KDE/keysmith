/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef MODEL_INPUT_H
#define MODEL_INPUT_H

#include "../account/account.h"
#include "../validators/countervalidator.h"
#include "../validators/datetimevalidator.h"

#include <QDateTime>
#include <QObject>
#include <QString>

#include <optional>

namespace model
{
    class AccountInput : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(model::AccountInput::TokenType type READ type WRITE setType NOTIFY typeChanged)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
        Q_PROPERTY(QString issuer READ issuer WRITE setIssuer NOTIFY issuerChanged)
        Q_PROPERTY(QString secret READ secret WRITE setSecret NOTIFY secretChanged)
        Q_PROPERTY(uint tokenLength READ tokenLength WRITE setTokenLength NOTIFY tokenLengthChanged)
        Q_PROPERTY(uint timeStep READ timeStep WRITE setTimeStep NOTIFY timeStepChanged)
        Q_PROPERTY(model::AccountInput::TOTPAlgorithm algorithm READ algorithm WRITE setAlgorithm NOTIFY algorithmChanged)
        Q_PROPERTY(QString epoch READ epoch WRITE setEpoch NOTIFY epochChanged)
        Q_PROPERTY(bool checksum READ checksum WRITE setChecksum NOTIFY checksumChanged)
        Q_PROPERTY(QString counter READ counter NOTIFY counterChanged)
        Q_PROPERTY(uint truncationOffset READ truncationOffset NOTIFY truncationChanged)
        Q_PROPERTY(bool fixedTruncation READ fixedTruncation NOTIFY truncationChanged)
    public:
        enum TOTPAlgorithm {
            Sha1, Sha256, Sha512
        };
        Q_ENUM(TOTPAlgorithm)
        enum TokenType {
            Hotp, Totp
        };
        Q_ENUM(TokenType)
        AccountInput(QObject *parent = nullptr);
        void createNewAccount(accounts::AccountStorage *storage) const;
        Q_INVOKABLE void reset(void);
    public:
        TokenType type(void) const;
        void setType(model::AccountInput::TokenType type);
        QString name(void) const;
        void setName(const QString &name);
        QString issuer(void) const;
        void setIssuer(const QString &issuer);
        QString secret(void) const;
        void setSecret(const QString &secret);
        uint tokenLength(void) const;
        void setTokenLength(uint tokenLength);
        uint timeStep(void) const;
        void setTimeStep(uint timeStep);
        TOTPAlgorithm algorithm(void) const;
        void setAlgorithm(model::AccountInput::TOTPAlgorithm algorithm);
        QString epoch(void) const;
        void setEpoch(const QString &epoch);
        bool checksum(void) const;
        void setChecksum(bool checksum);
        QString counter(void) const;
        void setCounter(quint64 counter);
        Q_INVOKABLE void setCounter(const QString &counter, validators::UnsignedLongValidator *validator);
        uint truncationOffset(void) const;
        bool fixedTruncation(void) const;
        Q_INVOKABLE void setTruncationOffset(uint truncationOffset);
        Q_INVOKABLE void setDynamicTruncation(void);
    Q_SIGNALS:
        void typeChanged(void);
        void nameChanged(void);
        void issuerChanged(void);
        void secretChanged(void);
        void tokenLengthChanged(void);
        void timeStepChanged(void);
        void algorithmChanged(void);
        void epochChanged(void);
        void checksumChanged(void);
        void counterChanged(void);
        void truncationChanged(void);
    private:
        TokenType m_type;
        QString m_name;
        QString m_issuer;
        QString m_secret;
        uint m_tokenLength;
        uint m_timeStep;
        TOTPAlgorithm m_algorithm;
        QString m_epoch;
        QDateTime m_epochValue;
        bool m_checksum;
        QString m_counter;
        quint64 m_counterValue;
        std::optional<uint> m_truncation;
    };
}

#endif
