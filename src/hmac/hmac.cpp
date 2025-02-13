/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "hmac.h"
#include "../logging_p.h"

KEYSMITH_LOGGER(logger, ".hmac")

static QByteArray hmac_stage(QCryptographicHash::Algorithm algorithm,
                             char *const keyBuf,
                             int ksize,
                             int blockSize,
                             const char fillPad,
                             const char xorKey,
                             const QByteArray &message)
{
    QCryptographicHash hash(algorithm);
    int count;
    for (count = 0; count < ksize; ++count) {
        keyBuf[count] ^= xorKey;
    }

    hash.addData(QByteArrayView(keyBuf, count));

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

std::optional<uint> outputSize(QCryptographicHash::Algorithm algorithm)
{
    switch (algorithm) {
    case QCryptographicHash::Md4:
    case QCryptographicHash::Md5:
        return std::optional<uint>(16ULL);
    case QCryptographicHash::Sha1:
        return std::optional<uint>(20ULL);
    case QCryptographicHash::Sha224:
    case QCryptographicHash::RealSha3_224:
    case QCryptographicHash::Keccak_224:
        return std::optional<int>(28UL);
    case QCryptographicHash::Sha256:
    case QCryptographicHash::RealSha3_256:
    case QCryptographicHash::Keccak_256:
        return std::optional<int>(32UL);
    case QCryptographicHash::Sha384:
    case QCryptographicHash::RealSha3_384:
    case QCryptographicHash::Keccak_384:
        return std::optional<int>(48UL);
    case QCryptographicHash::Sha512:
    case QCryptographicHash::RealSha3_512:
    case QCryptographicHash::Keccak_512:
        return std::optional<uint>(64UL);
    default:
        return std::nullopt;
    }
}

bool validateKeySize(QCryptographicHash::Algorithm algorithm, int ksize, bool requireSaneKeyLength)
{
    std::optional<int> maybeOutputSize = outputSize(algorithm);
    return maybeOutputSize && ksize >= 0 && (ksize >= (*maybeOutputSize) || !requireSaneKeyLength);
}

std::optional<QByteArray> compute(QCryptographicHash::Algorithm algorithm, char *rawKey, int ksize, const QByteArray &message, bool requireSaneKeyLength)
{
    if (!rawKey) {
        return std::nullopt;
    }

    std::optional<int> maybeBlockSize = blockSize(algorithm);
    if (!maybeBlockSize) {
        qCDebug(logger) << "Unable to determine block size for hashing algorithm:" << algorithm;
        return std::nullopt;
    }

    if (!validateKeySize(algorithm, ksize, requireSaneKeyLength)) {
        qCDebug(logger) << "Invalid HMAC key size:" << ksize << "for hashing algorithm:" << algorithm
                        << "Sane key length requirements apply:" << requireSaneKeyLength;
        return std::nullopt;
    }

    int bs = *maybeBlockSize;
    QByteArray hashedKey;

    if (ksize > bs) {
        QCryptographicHash khash(algorithm);
        khash.addData(QByteArrayView(rawKey, ksize));
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
