/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef MODEL_QR_CODE_H
#define MODEL_QR_CODE_H

#include "input.h"
#include "../uri/uri.h"

#include <QByteArray>
#include <QObject>
#include <QString>

#include <optional>

namespace model
{
    class QrParameters
    {
    public:
        static std::optional<QrParameters> parse(const QByteArray &qrCode);
        static std::optional<QrParameters> parse(const QString &qrCode);
        static std::optional<QrParameters> from(const uri::QrParts &parts);
        void populate(AccountInput *input) const;
    private:
        explicit QrParameters(AccountInput::TokenType type, const QString &name, const QString &issuer,
                              const QString &secret, uint tokenLength, quint64 counter, uint timeStep,
                              AccountInput::TOTPAlgorithm algorithm);
    private:
        const AccountInput::TokenType m_type;
        const QString m_name;
        const QString m_issuer;
        const QString m_secret;
        const uint m_tokenLength;
        const quint64 m_counter;
        const uint m_timeStep;
        const AccountInput::TOTPAlgorithm m_algorithm;
    };
}

#endif
