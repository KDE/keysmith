/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "base32.h"

static const QChar alphaMinLowerCase(QLatin1Char('a'));
static const QChar alphaMaxLowerCase(QLatin1Char('z'));
static const QChar alphaMinUpperCase(QLatin1Char('A'));
static const QChar alphaMaxUpperCase(QLatin1Char('Z'));
static const QChar numMin(QLatin1Char('2'));
static const QChar numMax(QLatin1Char('7'));
static const QChar pad(QLatin1Char('='));

static inline bool checkInputRange(const QString &encoded, int from, int until)
{
    /*
     * from should be between 0 (inclusive) and size (exclusive)
     * until should be between from (inclusive) and size (inclusive)
     * total range size (until - from) should be a multiple of 8 or it is not valid base32
     */
    int size = encoded.size();
    return from >= 0 && from <= size && until >= from && until <= size && ((until - from) % 8) == 0;
}

static std::optional<int> decode(const QChar &chr)
{
    if (chr >= alphaMinLowerCase && chr <= alphaMaxLowerCase) {
        return std::optional<int>(chr.toLatin1() - alphaMinLowerCase.toLatin1());
    }
    if (chr >= alphaMinUpperCase && chr <= alphaMaxUpperCase) {
        return std::optional<int>(chr.toLatin1() - alphaMinUpperCase.toLatin1());
    }
    if (chr >= numMin && chr <= numMax) {
        return std::optional<int>(26 + chr.toLatin1() - numMin.toLatin1());
    }

    if (chr >= pad) {
        return std::optional<int>(0);
    }

    return std::nullopt;
}

static std::optional<quint64> decode(const QString &encoded, int index)
{
    quint64 result = 0ULL;

    for (int i = 0; i < 8; ++i) {
        std::optional<int> v = decode(encoded[index + i]);
        if (v) {
            result = (result << 5) | *v;
        } else {
            // TODO warn about this
            return std::nullopt;
        }
    }

    return std::optional<quint64>(result);
}

static std::optional<size_t> decode(const QString &encoded, int index, int end, int padding, size_t offset, size_t capacity, char * const output)
{
    Q_ASSERT_X(offset <= capacity, Q_FUNC_INFO, "invalid offset into output buffer");
    Q_ASSERT_X(end >= 0 && end <= encoded.size(), Q_FUNC_INFO, "end of encoded data should be valid");
    Q_ASSERT_X(padding >= 0 && padding <= end, Q_FUNC_INFO, "padding index should be valid");
    Q_ASSERT_X(index >= 0 && index <= padding && ((end - index) % 8) == 0, Q_FUNC_INFO, "index should be valid");

    size_t group;

    switch ((index + 8) - padding)
    {
        case 2:
        case 5:
        case 7:
            Q_ASSERT_X(false, Q_FUNC_INFO, "invalid amount of padding should have been caught by previous validation");
            return std::nullopt;
        case 1:
            group = 4;
            break;
        case 3:
            group = 3;
            break;
        case 4:
            group = 2;
            break;
        case 6:
            group = 1;
            break;
        default: // no padding (yet) for the group at the given index: there are 8 or more bytes left
            group = 5;
            break;
    }

    Q_ASSERT_X((capacity - offset) >= group, Q_FUNC_INFO, "offset/output group too big for output buffer size");

    std::optional<quint64> bits = decode(encoded, index);

    Q_ASSERT_X(bits, Q_FUNC_INFO, "invalid input should have been caught by prior validation");

    quint64 value = *bits;
    for (size_t i = 0; i < group; ++i) {
        output[offset + i] = (char) ((value >> (32ULL - i * 8ULL)) & 0xFFULL);
    }

    return std::optional<size_t>(group);
}

static inline bool isBase32(const QChar &c)
{
    return (c >= alphaMinLowerCase && c <= alphaMaxLowerCase) || (c >= alphaMinUpperCase && c <= alphaMaxUpperCase) || (c >= numMin && c <= numMax);
}

static bool isPaddingValid(const QString &encoded, int paddingIndex, int amount)
{
    static const int padMasks[7] = {
        0x7, // 8 - 1 padding -> 7 * 5 - 32 bits -> 3 trailing bits: mask 0x7
        0x0, // 8 - 2 padding -> invalid
        0x1, // 8 - 3 padding -> 5 * 5 - 24 bits -> 1 trailing bit : mask 0x1
        0xF, // 8 - 4 padding -> 4 * 5 - 16 bits -> 4 trailing bits: mask 0xF
        0x0, // 8 - 5 padding -> invalid
        0x3, // 8 - 6 padding -> 2 * 5 -  8 bits -> 2 trailing bits: mask 0x3
        0x0  // 8 - 7 padding -> invalid
    };

    if (amount == 0) {
        return true;
    }

    if (amount >= 8) {
        return false;
    }

    Q_ASSERT_X(paddingIndex >= 0, Q_FUNC_INFO, "invalid amount of padding should have been caught by previous validation");

    const QChar c = encoded[paddingIndex - 1];
    Q_ASSERT_X(c != pad, Q_FUNC_INFO, "invalid amount of padding should have been caught by previous validation");

    /*
     * Check if the amount of padding corresponds to a known (valid) input 'group' size
     * by looking up the mask for the last character before padding (0 = invalid)
     */
    int p = padMasks[amount - 1];
    if (p == 0) {
        return false;
    }

    std::optional<int> d = decode(c);
    Q_ASSERT_X(d, Q_FUNC_INFO, "invalid input should have been caught by prior validation");

    /*
     * check if there are no trailing bits,
     * i.e. the last character before padding does not encode bits that are not whitelisted by the mask
     */
    return ((*d) & p) == 0;

}

static std::optional<int> isBase32(const QString &encoded, int from, int until)
{
    if (!checkInputRange(encoded, from, until)) {
        return std::nullopt;
    }

    int paddingIndex = until;
    for (int i = from; i < until; ++i) {
        const QChar at = encoded[i];
        if (at == pad) {
            if (paddingIndex == until) {
                paddingIndex = i;
            }
        } else {
            /*
             * Reject input if:
             * - padding has 'started' but the current character is not the padding character
             * - the current character is not a (valid) value character
             */
            if (paddingIndex < until || !isBase32(at)) {
                return std::nullopt;
            }
        }
    }

    int amount = until - paddingIndex;

    return isPaddingValid(encoded, paddingIndex, amount) ? std::optional<int>(paddingIndex) : std::nullopt;
}

static inline size_t determineCapacity(size_t encodedBytes, size_t accountFor, size_t lastBytes)
{
    return 5 * (encodedBytes - accountFor) / 8  + lastBytes;
}

static size_t requiredCapacity(int paddingIndex, int from, int until)
{
    // based on the amount of padding, determine the exact size of the encoded data
    int size = paddingIndex - from;
    switch (until - paddingIndex) {
    case 0:
        return determineCapacity(size, 0, 0);
    case 1:
        return determineCapacity(size, 7, 4);
    case 3:
        return determineCapacity(size, 5, 3);
    case 4:
        return determineCapacity(size, 4, 2);
    case 6:
        return determineCapacity(size, 2, 1);
    default:
        Q_ASSERT_X(false, Q_FUNC_INFO, "invalid input size/amount of padding should have been caught by previous validation");
        return 0;
    }
}

namespace base32
{
    std::optional<size_t> validate(const QString &encoded, int from, int until)
    {
        int max = until == -1 ? encoded.size() : until;
        if (!checkInputRange(encoded, from, max)) {
            return std::nullopt;
        }

        std::optional<int> padding = isBase32(encoded, from, max);
        return padding ? std::optional<size_t>(requiredCapacity(*padding, from, max)) : std::nullopt;
    }

    std::optional<size_t> decode(const QString &encoded, char * const out, size_t outlen, int from, int until)
    {
        int max = until == -1 ? encoded.size() : until;
        if (!checkInputRange(encoded, from, max)) {
            // TODO warn about this
            return std::nullopt;
        }

        std::optional<int> padding = isBase32(encoded, from, max);
        if (!padding) {
            // TODO warn about this
            return std::nullopt;
        }

        size_t needed = requiredCapacity(*padding, from, max);
        if (outlen < needed) {
            // TODO warn about this
            return std::nullopt;
        }

        int index;
        size_t decoded = 0;
        for(index = from; index < max && decoded < needed; index += 8) {
            std::optional<size_t> group = decode(encoded, index, max, *padding, decoded, needed, out);
            Q_ASSERT_X(group, Q_FUNC_INFO, "input should have been fully validated; decoding should succeed");
            decoded += *group;
        }

        Q_ASSERT_X(decoded == needed, Q_FUNC_INFO, "number of bytes decoded should match expected output capacity required");
        Q_ASSERT_X(index == max, Q_FUNC_INFO, "number of characters decoded should match end of the input range exactly");

        return std::optional<size_t>(decoded);
    }

    std::optional<QByteArray> decode(const QString &encoded)
    {
        std::optional<QByteArray> result = std::nullopt;
        std::optional<size_t> capacity = validate(encoded);

        if (!capacity) {
            // TODO warn about this
            return std::nullopt;
        }

        QByteArray decoded;
        decoded.reserve((int) *capacity);
        decoded.resize((int) *capacity);
        if (decode(encoded, decoded.data(), *capacity)) {
            result.emplace(decoded);
        }
        // TODO warn if not

        return result;
    }
}
