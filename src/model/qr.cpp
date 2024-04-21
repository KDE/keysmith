/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "qr.h"

#include "../base32/base32.h"

#include <limits>

static std::optional<quint64> parseUnsigned(const QString &value, const std::function<bool(qulonglong)> &check)
{
    bool ok = false;
    quint64 v = value.toULongLong(&ok, 10);

    return ok && check(v) ? std::optional<quint64>(v) : std::nullopt;
}

static std::optional<uint> parseUnsigned(const QString &value, uint valueIfEmpty, const std::function<bool(quint64)> &check)
{
    if (value.isEmpty()) {
        return std::optional<uint>(valueIfEmpty);
    }

    const auto r = parseUnsigned(value, check);
    return r ? std::optional<uint>((uint)*r) : std::nullopt;
}

static std::function<bool(quint64)> positiveUpTo(quint64 max)
{
    return std::function<bool(quint64)>([max](quint64 v) -> bool {
        return v >= 1ULL && v <= max;
    });
}

static std::optional<QString> checkNonEmptyString(const QString &value, const std::function<bool(QString &)> &check)
{
    QString v(value);
    return v.isEmpty() || !check(v) ? std::nullopt : std::optional<QString>(v);
}

static std::function<bool(QString &)> usableName(const QChar reserved = QLatin1Char('\0'))
{
    return std::function<bool(QString &)>([reserved](QString &v) -> bool {
        for (const auto c : v) {
            if (!c.isPrint() || c == reserved) {
                return false;
            }
        }

        return true;
    });
}

namespace model
{

static std::optional<AccountInput::TokenType> convertType(uri::QrParts::Type type)
{
    switch (type) {
    case uri::QrParts::Type::Hotp:
        return std::optional<AccountInput::TokenType>(AccountInput::TokenType::Hotp);
    case uri::QrParts::Type::Totp:
        return std::optional<AccountInput::TokenType>(AccountInput::TokenType::Totp);
    default:
        Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown/unsupported otpauth token type?");
        return std::nullopt;
    }
}

static std::optional<AccountInput::TOTPAlgorithm> convertAlgorithm(const QString &algorithm)
{
    static const QString sha1 = QStringLiteral("sha1");
    static const QString sha256 = QStringLiteral("sha256");
    static const QString sha512 = QStringLiteral("sha512");

    if (algorithm.isEmpty()) {
        return std::optional<AccountInput::TOTPAlgorithm>(AccountInput::TOTPAlgorithm::Sha1);
    }

    const auto lower = algorithm.toLower();
    if (lower == sha1) {
        return std::optional<AccountInput::TOTPAlgorithm>(AccountInput::TOTPAlgorithm::Sha1);
    }
    if (lower == sha256) {
        return std::optional<AccountInput::TOTPAlgorithm>(AccountInput::TOTPAlgorithm::Sha256);
    }
    if (lower == sha512) {
        return std::optional<AccountInput::TOTPAlgorithm>(AccountInput::TOTPAlgorithm::Sha512);
    }

    return std::nullopt;
}

QrParameters::QrParameters(AccountInput::TokenType type,
                           const QString &name,
                           const QString &issuer,
                           const QString &secret,
                           uint tokenLength,
                           quint64 counter,
                           uint timeStep,
                           AccountInput::TOTPAlgorithm algorithm)
    : m_type(type)
    , m_name(name)
    , m_issuer(issuer)
    , m_secret(secret)
    , m_tokenLength(tokenLength)
    , m_counter(counter)
    , m_timeStep(timeStep)
    , m_algorithm(algorithm)
{
}

std::optional<QrParameters> QrParameters::parse(const QByteArray &qrCode)
{
    const auto parts = uri::QrParts::parse(qrCode);
    return parts ? from(*parts) : std::nullopt;
}

std::optional<QrParameters> QrParameters::parse(const QString &qrCode)
{
    const auto parts = uri::QrParts::parse(qrCode);
    return parts ? from(*parts) : std::nullopt;
}

std::optional<QrParameters> QrParameters::from(const uri::QrParts &parts)
{
    const auto type = convertType(parts.type());
    const auto algorithm = convertAlgorithm(parts.algorithm());
    const auto timeStep = parseUnsigned(parts.timeStep(), 30U, positiveUpTo(std::numeric_limits<uint>::max()));

    const auto tokenLength = parseUnsigned(parts.tokenLength(), 6U, [](qulonglong v) -> bool {
        return v >= 6ULL && v <= 10ULL;
    });

    const auto counter =
        parts.counter().isEmpty() ? std::optional<quint64>(0ULL) : parseUnsigned(parts.counter(), positiveUpTo(std::numeric_limits<quint64>::max()));

    const auto secret = checkNonEmptyString(parts.secret(), [](QString &v) -> bool {
        while ((v.size() % 8) != 0) {
            v += QLatin1Char('=');
        }

        return (bool)base32::validate(v);
    });

    const auto name = parts.name().isEmpty() ? std::optional<QString>(QString()) : checkNonEmptyString(parts.name(), usableName());

    const auto issuer = parts.issuer().isEmpty() ? std::optional<QString>(QString()) : checkNonEmptyString(parts.issuer(), usableName(QLatin1Char(':')));

    return type && algorithm && timeStep && tokenLength && counter && secret && name && issuer
        ? std::optional<QrParameters>(QrParameters(*type, *name, *issuer, *secret, *tokenLength, *counter, *timeStep, *algorithm))
        : std::nullopt;
}

void QrParameters::populate(AccountInput *input) const
{
    if (!input) {
        Q_ASSERT_X(input, Q_FUNC_INFO, "Input must be provided");
        return;
    }

    input->setType(m_type);
    input->setName(m_name);
    input->setIssuer(m_issuer);
    input->setSecret(m_secret);
    input->setTokenLength(m_tokenLength);
    input->setCounter(m_counter);
    input->setTimeStep(m_timeStep);
    input->setAlgorithm(m_algorithm);
}
}
