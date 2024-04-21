/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef OTPAUTH_URI_H
#define OTPAUTH_URI_H

#include <QByteArray>
#include <QObject>
#include <QString>

#include <optional>

namespace uri
{
/*
 * A forgiving percent encoding "decoder" which does not get confused by invalid/malformed input
 * (In contrast to QByteArray::fromPercentEncoding()).
 */
std::optional<QByteArray> fromPercentEncoding(const QByteArray &encoded);
std::optional<QString> decodePercentEncoding(const QByteArray &utf8Data);

class QrParts
{
public:
    enum Type { Totp, Hotp };
    static std::optional<QrParts> parse(const QByteArray &qrCode);
    static std::optional<QrParts> parse(const QString &qrCode);

public:
    Type type(void) const;
    QString algorithm(void) const;
    QString timeStep(void) const;
    QString tokenLength(void) const;
    QString counter(void) const;
    QString secret(void) const;
    QString name(void) const;
    QString issuer(void) const;

private:
    explicit QrParts(Type type,
                     const QString &name,
                     const QString &issuer,
                     const QString &secret,
                     const QString &tokenLength,
                     const QString &counter,
                     const QString &timeStep,
                     const QString &algorithm);

private:
    const Type m_type;
    const QString m_name;
    const QString m_issuer;
    const QString m_secret;
    const QString m_tokenLength;
    const QString m_counter;
    const QString m_timeStep;
    const QString m_algorithm;
};
}

#endif
