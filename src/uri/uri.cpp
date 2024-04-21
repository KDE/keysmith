/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "uri.h"

#include "../base32/base32.h"
#include "../logging_p.h"

#include <QGlobalStatic>
#include <QScopedPointer>
#include <QStringDecoder>

KEYSMITH_LOGGER(logger, ".uri")

namespace uri
{
static bool isHexDigit(const char digit)
{
    return (digit >= '0' && digit <= '9') || (digit >= 'A' && digit <= 'F') || (digit >= 'a' && digit <= 'f');
}

std::optional<QByteArray> fromPercentEncoding(const QByteArray &encoded)
{
    QByteArray decoded(encoded);

    int index = 0;
    for (index = decoded.indexOf('%', index); index >= 0; index = decoded.indexOf('%', index + 1)) {
        if (decoded.size() <= (index + 2) || !isHexDigit(decoded[index + 1]) || !isHexDigit(decoded[index + 2])) {
            return std::nullopt;
        }

        QByteArray substitute = QByteArray::fromHex(decoded.mid(index + 1, 2));
        decoded.replace(index, 3, substitute);
    }

    return std::optional<QByteArray>(decoded);
}

static std::optional<QString> convertUtf8(const QByteArray &data)
{
    QStringDecoder codec(QStringDecoder::Utf8);
    QString result = codec.decode(data);
    return !codec.hasError() ? std::optional<QString>(result) : std::nullopt;
}

std::optional<QString> decodePercentEncoding(const QByteArray &utf8Data)
{
    const auto decoded = fromPercentEncoding(utf8Data);
    return decoded ? convertUtf8(*decoded) : std::nullopt;
}

static bool tryDecodeParam(const QByteArray &param, const QByteArray &actual, const QByteArray &value, QByteArray &uri, QString &oldValue, bool &error)
{
    bool skipped = true;
    if (error || actual != param) {
        return skipped;
    }

    uri.remove(0, value.size());
    skipped = false;

    if (!oldValue.isNull()) {
        qCDebug(logger) << "Found duplicate parameter" << param;
        error = true;
        return skipped;
    }

    const auto result = decodePercentEncoding(value);
    if (!result) {
        qCDebug(logger) << "Failed to decode" << param << "Invalid URI encoding or malformed UTF-8";
        error = true;
        return skipped;
    }

    oldValue = *result;
    return skipped;
}

std::optional<QrParts> QrParts::parse(const QByteArray &qrCode)
{
    static const QByteArray schemePrefix("otpauth://");
    static const QByteArray totpType("totp");
    static const QByteArray hotpType("hotp");
    static const QByteArray issuerParam("issuer");
    static const QByteArray secretParam("secret");
    static const QByteArray algorithmParam("algorithm");
    static const QByteArray tokenLengthParam("digits");
    static const QByteArray timeStepParam("period");
    static const QByteArray counterParam("counter");

    QByteArray uri(qrCode);
    if (!uri.startsWith(schemePrefix)) {
        qCDebug(logger) << "Unexpected format: URI does not start with:" << schemePrefix;
        return std::nullopt;
    }

    uri.remove(0, schemePrefix.size());
    if (uri.size() < 4) {
        qCDebug(logger) << "No token type found: URI too short";
        return std::nullopt;
    }

    QByteArray typeField = uri.mid(0, 4);
    if (typeField != totpType && typeField != hotpType) {
        qCDebug(logger) << "Invalid token type found";
        return std::nullopt;
    }

    Type type = typeField == totpType ? Type::Totp : Type::Hotp;

    uri.remove(0, 4);
    int paramOffset = uri.indexOf('?');

    if (paramOffset < 0) {
        qCDebug(logger) << "No token parameters found: URI too short";
        return std::nullopt;
    }

    QString issuer;
    QString name(QLatin1String(""));

    if (uri[0] == '/') {
        QByteArray issuerNameField = uri.mid(1, paramOffset - 1);

        int colonOffset = issuerNameField.indexOf(':');
        int encodedColonOffset = issuerNameField.indexOf(QByteArray("%3A"));

        QByteArray issuerField;
        QByteArray nameField = issuerNameField;

        if (colonOffset >= 0 || encodedColonOffset >= 0) {
            if (colonOffset >= 0 && (colonOffset < encodedColonOffset || encodedColonOffset < 0)) {
                issuerField = issuerNameField.mid(0, colonOffset);
                nameField = issuerNameField.mid(colonOffset + 1);
            } else {
                issuerField = issuerNameField.mid(0, encodedColonOffset);
                nameField = issuerNameField.mid(encodedColonOffset + 3);
            }

            const auto decodedIssuer = uri::decodePercentEncoding(issuerField);
            if (!decodedIssuer) {
                qCDebug(logger) << "Failed to decode issuer: invalid URI encoding or malformed UTF-8";
                return std::nullopt;
            }

            issuer = *decodedIssuer;
        }

        const auto decodedName = uri::decodePercentEncoding(nameField);
        if (!decodedName) {
            qCDebug(logger) << "Failed to decode name: invalid URI encoding or malformed UTF-8";
            return std::nullopt;
        }

        name = *decodedName;

        uri.remove(0, paramOffset);
    }

    if (uri[0] != '?') {
        qCDebug(logger) << "No token parameters found: expected to find:" << '?';
        return std::nullopt;
    }

    QString secret;
    QString counter;
    QString timeStep;
    QString algorithm;
    QString tokenLength;
    QString otherIssuer;
    while (uri.size() > 1) {
        uri.remove(0, 1);
        QByteArray param;
        int valueOffset = uri.indexOf('=');
        switch (valueOffset) {
        case -1:
            qCDebug(logger) << "No parameter value found: URI too short";
            return std::nullopt;
        case 0:
            qCDebug(logger) << "Found a parameter value without a name";
            return std::nullopt;
        default:
            param = uri.mid(0, valueOffset);
            uri.remove(0, valueOffset + 1);
            break;
        }

        bool error = false;
        int nextKeyOffset = uri.indexOf('&');
        QByteArray value = uri.mid(0, nextKeyOffset);
        if (tryDecodeParam(secretParam, param, value, uri, secret, error) && tryDecodeParam(issuerParam, param, value, uri, otherIssuer, error)
            && tryDecodeParam(tokenLengthParam, param, value, uri, tokenLength, error) && tryDecodeParam(timeStepParam, param, value, uri, timeStep, error)
            && tryDecodeParam(counterParam, param, value, uri, counter, error) && tryDecodeParam(algorithmParam, param, value, uri, algorithm, error)) {
            qCDebug(logger) << "Invalid/unsupported parameter found";
            return std::nullopt;
        }

        if (error) {
            return std::nullopt;
        }
    }

    if (secret.isEmpty()) {
        qCDebug(logger) << "No token secret found: expected to find:" << secretParam << "parameter";
        return std::nullopt;
    }

    return std::optional<QrParts>(QrParts(type,
                                          name,
                                          issuer.isNull() || (issuer.isEmpty() && !otherIssuer.isEmpty()) ? otherIssuer : issuer,
                                          secret,
                                          tokenLength,
                                          counter,
                                          timeStep,
                                          algorithm));
}

std::optional<QrParts> QrParts::parse(const QString &qrCode)
{
    return parse(qrCode.toUtf8());
}

QrParts::Type QrParts::type(void) const
{
    return m_type;
}

QString QrParts::algorithm(void) const
{
    return m_algorithm;
}

QString QrParts::timeStep(void) const
{
    return m_timeStep;
}

QString QrParts::tokenLength(void) const
{
    return m_tokenLength;
}

QString QrParts::counter(void) const
{
    return m_counter;
}

QString QrParts::secret(void) const
{
    return m_secret;
}

QString QrParts::name(void) const
{
    return m_name;
}

QString QrParts::issuer(void) const
{
    return m_issuer;
}

QrParts::QrParts(Type type,
                 const QString &name,
                 const QString &issuer,
                 const QString &secret,
                 const QString &tokenLength,
                 const QString &counter,
                 const QString &timeStep,
                 const QString &algorithm)
    : //, const Warnings &warnings) :
    m_type(type)
    , m_name(name)
    , m_issuer(issuer)
    , m_secret(secret)
    , m_tokenLength(tokenLength)
    , m_counter(counter)
    , m_timeStep(timeStep)
    , m_algorithm(algorithm) //, m_warnings(warnings)
{
}
}
