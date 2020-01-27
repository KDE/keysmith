/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "util.h"

namespace oath
{
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
}
