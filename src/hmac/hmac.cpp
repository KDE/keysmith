/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "hmac.h"

static QByteArray hmac_stage(QCryptographicHash::Algorithm algorithm, char * const keyBuf, int ksize, int blockSize, const char fillPad, const char xorKey, const QByteArray &message)
{
    QCryptographicHash hash(algorithm);
    int count;
    for (count = 0; count < ksize; ++count) {
        keyBuf[count] ^= xorKey;
    }

    hash.addData(keyBuf, count);

    if (ksize < blockSize) {
        QByteArray pad(blockSize - ksize, fillPad);
        hash.addData(pad);
    }

    hash.addData(message);

    return hash.result();
}

namespace hmac
{
    std::optional<int> blockSize(QCryptographicHash::Algorithm algorithm)
    {
        switch (algorithm) {
        case QCryptographicHash::Md4:
        case QCryptographicHash::Md5:
        case QCryptographicHash::Sha1:
        case QCryptographicHash::Sha224:
        case QCryptographicHash::Sha256:
            return std::optional<int>(64ULL);
        case QCryptographicHash::Sha384:
        case QCryptographicHash::Sha512:
            return std::optional<int>(128ULL);
        case QCryptographicHash::RealSha3_224:
        case QCryptographicHash::Keccak_224:
            return std::optional<int>(144ULL);
        case QCryptographicHash::RealSha3_256:
        case QCryptographicHash::Keccak_256:
            return std::optional<int>(136ULL);
        case QCryptographicHash::RealSha3_384:
        case QCryptographicHash::Keccak_384:
            return std::optional<int>(104ULL);
        case QCryptographicHash::RealSha3_512:
        case QCryptographicHash::Keccak_512:
            return std::optional<int>(72ULL);
        default:
            return std::nullopt;
        }
    }

    bool validateKeySize(QCryptographicHash::Algorithm algorithm, int ksize, bool requireSaneKeyLength)
    {
        std::optional<int> maybeBlockSize = blockSize(algorithm);
        return maybeBlockSize && ksize >= 0 && (ksize >= (*maybeBlockSize) || !requireSaneKeyLength);
    }

    std::optional<QByteArray> compute(QCryptographicHash::Algorithm algorithm, char * rawKey, int ksize, const QByteArray &message, bool requireSaneKeyLength)
    {
        if (!rawKey) {
            return std::nullopt;
        }

        std::optional<int> maybeBlockSize = blockSize(algorithm);
        if (!maybeBlockSize) {
            // TODO warn
            return std::nullopt;
        }
        if (!validateKeySize(algorithm, ksize, requireSaneKeyLength)) {
            // TODO warn
            return std::nullopt;
        }

        int bs = *maybeBlockSize;
        QByteArray hashedKey;

        if (ksize > bs) {
            QCryptographicHash khash(algorithm);
            khash.addData(rawKey, ksize);
            hashedKey = khash.result();
            rawKey = hashedKey.data();
            ksize = hashedKey.size();
        }

        QByteArray inner = hmac_stage(algorithm, rawKey, ksize, bs, '\x36', '\x36', message);
        QByteArray result = hmac_stage(algorithm, rawKey, ksize, bs, '\x5c', '\x36' ^ '\x5c', inner);

        hashedKey.fill('\x0');
        return std::optional<QByteArray>(result);
    }
}
