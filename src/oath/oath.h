/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef OATH_H
#define OATH_H

#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>

#include <optional>

namespace oath
{
    class Encoder
    {
    public:
        Encoder(uint tokenLength, bool addChecksum = false);
        virtual ~Encoder();
        virtual QString encode(quint32) const;
        uint tokenLength(void) const;
        bool checksum(void) const;
        static quint32 reduceMod10(quint32 value, uint tokenLength);
    private:
        static constexpr const quint32 powerTable[10] = {
            1, 10, 100, 1'000, 10'000, 100'000, 1'000'000, 10'000'000, 100'000'000, 1'000'000'000
        };
        const uint m_tokenLength;
        const bool m_addChecksum;
    };

    class Algorithm
    {
    public:
        static bool validate(const Encoder& encoder);
        static std::optional<Algorithm> usingDynamicTruncation(QCryptographicHash::Algorithm algorithm, uint tokenLength, bool addChecksum = false, bool requireSaneKeyLength = false);
        static std::optional<Algorithm> usingDynamicTruncation(QCryptographicHash::Algorithm algorithm, const Encoder &encoder, bool requireSaneKeyLength = false);
        static std::optional<Algorithm> usingTruncationOffset(QCryptographicHash::Algorithm algorithm, uint offset, const Encoder &encoder, bool requireSaneKeyLength = false);
        std::optional<QString> compute(quint64 counter, char * secretBuffer, int length) const;
    private:
        Algorithm(const Encoder &encoder, const std::function<quint32(QByteArray)> &truncation, QCryptographicHash::Algorithm algorithm, bool requireSaneKeyLength);
    private:
        const Encoder m_encoder;
        const std::function<quint32(QByteArray)> m_truncation;
        bool m_enforceKeyLength;
        const QCryptographicHash::Algorithm m_algorithm;
    };

    std::optional<quint64> count(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);
}

#endif
