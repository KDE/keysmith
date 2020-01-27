/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath.h"

#include "util.h"

#include "../hmac/hmac.h"

static QString encodeDefaults(quint32 value, uint tokenLength)
{
    Q_ASSERT_X(tokenLength >= 6, Q_FUNC_INFO, "token length should be at least 6 characters long");

    QString base;
    base.setNum(value, 10);

    QString prefix(QLatin1String(""));
    for (uint i = base.size(); i < tokenLength; ++i) {
        prefix += QLatin1Char('0');
    }

    return prefix + base;
}

static QString encodeDefaultsWithChecksum(quint32 value, uint tokenLength)
{
    QString prefix = encodeDefaults(value, tokenLength);

    QString check;
    check.setNum(oath::luhnChecksum(value, tokenLength), 10);

    return prefix + check;
}

static quint32 truncate(const QByteArray &hash, uint offset)
{
    Q_ASSERT_X(hash.size() >= 4, Q_FUNC_INFO, "hash output is too small");
    Q_ASSERT_X(offset <= (hash.size() - 4), Q_FUNC_INFO, "truncation offset is too large for the hash output");

    return ((((quint32) hash[offset]) & 0x7FUL) << 24)
        | ((((quint32) hash[offset + 1]) & 0xFFUL) << 16)
        | ((((quint32) hash[offset + 2]) & 0xFFUL) << 8)
        | (((quint32) hash[offset + 3]) & 0xFFUL);
}

static quint32 truncateDynamically(const QByteArray &hash)
{
    Q_ASSERT_X(hash.size() >= 20, Q_FUNC_INFO, "hash output is too small");
    return truncate(hash, ((uint) hash[hash.size() - 1]) & 0x0FUL);
}

namespace oath
{
    Encoder::Encoder(uint tokenLength, bool addChecksum) : m_tokenLength(tokenLength), m_addChecksum(addChecksum)
    {
    }

    Encoder::~Encoder()
    {
    }

    quint32 Encoder::reduceMod10(quint32 value, uint tokenLength)
    {
        /*
         * Skip modulo 10 reducation for tokens of 10 or more characters:
         * the value is already guaranteed to be in its modulo 10 reduced form, because 2^32 is less than 10^10.
         * This check also takes care of possible integer overflow, for the same reason.
         */
        return tokenLength <= 9 ? value % powerTable[tokenLength] : value;
    }

    QString Encoder::encode(quint32 value) const
    {
        value = reduceMod10(value, m_tokenLength);
        return m_addChecksum ? encodeDefaultsWithChecksum(value, m_tokenLength) : encodeDefaults(value, m_tokenLength);
    }

    uint Encoder::tokenLength(void) const
    {
        return m_tokenLength;
    }

    bool Encoder::checksum(void) const
    {
        return m_addChecksum;
    }

    bool Algorithm::validate(const Encoder &encoder)
    {
        // HOTP spec mandages a mimimum token length of 6 digits
        return encoder.tokenLength() >= 6;
    }

    std::optional<Algorithm> Algorithm::usingDynamicTruncation(QCryptographicHash::Algorithm algorithm, uint tokenLength, bool addChecksum, bool requireSaneKeyLength)
    {
        const Encoder encoder(tokenLength, addChecksum);
        return usingDynamicTruncation(algorithm, encoder, requireSaneKeyLength);
    }

    std::optional<Algorithm> Algorithm::usingDynamicTruncation(QCryptographicHash::Algorithm algorithm, const Encoder &encoder, bool requireSaneKeyLength)
    {
        std::optional<uint> digestSize = outputSize(algorithm);
        if (!digestSize) {
            // TODO warn about this
            return std::nullopt;
        }

        if ((*digestSize) < 20) {
            // TODO warn about this
            return std::nullopt;
        }

        if (!validate(encoder)) {
            // TODO warn about this
            return std::nullopt;
        }

        const std::function<quint32(QByteArray)> truncation(truncateDynamically);
        return std::optional<Algorithm>(Algorithm(encoder, truncation, algorithm, requireSaneKeyLength));
    }

    std::optional<Algorithm> Algorithm::usingTruncationOffset(QCryptographicHash::Algorithm algorithm, uint offset, const Encoder &encoder, bool requireSaneKeyLength)
    {
        std::optional<uint> digestSize = outputSize(algorithm);
        if (!digestSize) {
            // TODO warn about this
            return std::nullopt;
        }

        if (offset >= ((*digestSize) - 4)) {
            // TODO warn about this
            return std::nullopt;
        }

        if (!validate(encoder)) {
            // TODO warn about this
            return std::nullopt;
        }

        const std::function<quint32(QByteArray)> truncation([offset](QByteArray bytes) -> quint32
        {
            return truncate(bytes, offset);
        });
        return std::optional<Algorithm>(Algorithm(encoder, truncation, algorithm, requireSaneKeyLength));
    }

    Algorithm::Algorithm(const Encoder &encoder, const std::function<quint32(QByteArray)> &truncation, QCryptographicHash::Algorithm algorithm, bool requireSaneKeyLength) :
        m_encoder(encoder), m_truncation(truncation), m_enforceKeyLength(requireSaneKeyLength), m_algorithm(algorithm)
    {
    }

    std::optional<QString> Algorithm::compute(quint64 counter, char * secretBuffer, int length) const
    {
        if (!secretBuffer) {
            return std::nullopt;
        }

        if (!hmac::validateKeySize(m_algorithm, length, m_enforceKeyLength)) {
            // TODO warn about this
            return std::nullopt;
        }

        QByteArray message;
        message.resize(8);

        for (int i = 0; i < 8; ++i) {
            message[i] = (char) ((counter >> (56 - i * 8)) & 0xFFULL);
        }

        std::optional<QByteArray> digest = hmac::compute(m_algorithm, secretBuffer, length, message, m_enforceKeyLength);
        if (digest) {
            quint32 result = m_truncation(*digest);
            result = Encoder::reduceMod10(result, m_encoder.tokenLength());
            return std::optional<QString>(m_encoder.encode(result));
        }
        // TODO warn if not

        return std::nullopt;
    }

    std::optional<quint64> count(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock)
    {
        qint64 epochMillis = epoch.toMSecsSinceEpoch();
        qint64 now = clock();

        if (now < epochMillis) {
            // TODO warn about this
            return std::nullopt;
        }

        if (timeStep == 0UL) {
            // TODO warn about this
            return std::nullopt;
        }

        quint64 msecs = ((quint64) (now - epochMillis));
        quint64 stepInMsecs = ((quint64) timeStep) * 1000ULL;
        return std::optional<quint64>(msecs / stepInMsecs);
    }
}
