/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "oath.h"

#include "../hmac/hmac.h"
#include "../logging_p.h"

KEYSMITH_LOGGER(logger, ".oath")

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
    Q_ASSERT_X(offset <= (((uint) hash.size()) - 4UL), Q_FUNC_INFO, "truncation offset is too large for the hash output");

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
         * Skip modulo 10 reduction for tokens of 10 or more characters:
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
        // HOTP spec mandates a minimum token length of 6 digits
        return encoder.tokenLength() >= 6;
    }

    bool Algorithm::validate(QCryptographicHash::Algorithm algorithm, const std::optional<uint> &offset)
    {
        /*
         * An nullopt offset indicates dynamic truncation.
         * Dynamic truncation works by taking the last nible and interpreting it as offset for truncation, i.e. it will always be <= 15.
         * Accounting for the last nibble (therefore last byte) assume a max truncation offset of 16 if dynamic truncation is used.
         */
        uint truncateAt = offset ? *offset : 16U;

        /*
         * The given algorithm must be supported/have a known digest size.
         * There must be at least 4 bytes available at the given truncation offset/limit.
         */
        std::optional<uint> digestSize = hmac::outputSize(algorithm);
        return digestSize && *digestSize >= 4U && (*digestSize - 4U) >= truncateAt;
    }

    std::optional<Algorithm> Algorithm::create(QCryptographicHash::Algorithm algorithm, const std::optional<uint> &offset, const Encoder &encoder, bool requireSaneKeyLength)
    {
        if(!validate(algorithm, offset)) {
            qCDebug(logger) << "Invalid algorithm:" << algorithm << "or incompatible with truncation offset:" << (offset ? *offset : 16U);
            return std::nullopt;
        }

        if (!validate(encoder)) {
            qCDebug(logger) << "Invalid token encoder";
            return std::nullopt;
        }

        std::function<quint32(QByteArray)> truncation(truncateDynamically);
        if (offset) {
            uint at = *offset;
            truncation = [at](QByteArray bytes) -> quint32
            {
                return truncate(bytes, at);
            };
        }

        return std::optional<Algorithm>(Algorithm(encoder, truncation, algorithm, requireSaneKeyLength));
    }

    std::optional<Algorithm> Algorithm::totp(QCryptographicHash::Algorithm algorithm, uint tokenLength, bool requireSaneKeyLength)
    {
        const Encoder encoder(tokenLength, false);
        return create(algorithm, std::nullopt, encoder, requireSaneKeyLength);
    }

    std::optional<Algorithm> Algorithm::hotp(const std::optional<uint> &offset, uint tokenLength, bool checksum, bool requireSaneKeyLength)
    {
        const Encoder encoder(tokenLength, checksum);
        return create(QCryptographicHash::Sha1, offset, encoder, requireSaneKeyLength);
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
            qCDebug(logger)
                << "Invalid key size:" << length << "for algorithm:" << m_algorithm
                << "Sane key length requirements apply:" << m_enforceKeyLength;
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

        qCDebug(logger) << "Failed to compute token";
        return std::nullopt;
    }

    uint luhnChecksum(quint32 value, uint digits)
    {
        static const uint lookupTable[10] = {
            0, // 0 * 2
            2, // 1 * 2
            4, // 2 * 2
            6, // 3 * 2
            8, // 4 * 2
            1, // 5 * 2 - 9
            3, // 6 * 2 - 9
            5, // 7 * 2 - 9
            7, // 8 * 2 - 9
            9, // 9 * 2 - 9
        };

        Q_ASSERT_X(digits > 0UL, Q_FUNC_INFO, "checksum cannot be computed over less than 1 digit");
        uint sum = 0UL;
        bool doubledMinus9 = true;
        for (uint d = 0UL; d < digits && value != 0UL; ++d) {
            uint position = value % 10UL;

            sum += doubledMinus9 ? lookupTable[position] : position;

            value /= 10UL;
            doubledMinus9 = !doubledMinus9;
        }

        sum = sum % 10ULL;
        return sum == 0UL ? 0UL : 10UL - sum;
    }

    std::optional<quint64> count(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock)
    {
        qint64 epochMillis = epoch.toMSecsSinceEpoch();
        qint64 now = clock();

        if (now < epochMillis) {
            qCDebug(logger) << "Unable to count time steps: epoch is in the future";
            return std::nullopt;
        }

        if (timeStep == 0UL) {
            qCDebug(logger) << "Unable to count time steps: invalid step size:" << timeStep;
            return std::nullopt;
        }

        quint64 msecs = ((quint64) (now - epochMillis));
        quint64 stepInMsecs = ((quint64) timeStep) * 1000ULL;
        return std::optional<quint64>(msecs / stepInMsecs);
    }
}
