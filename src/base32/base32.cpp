/*****************************************************************************
 * Copyright: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>                  *
 *                                                                           *
 * This project is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This project is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#include "base32.h"

#include "../oath_p.h"

#include <QtDebug>

#include <stdlib.h>
#include <string.h>

static inline size_t determineCapacity(size_t encodedBytes, size_t accountFor, size_t lastBytes)
{
    return 5 * (encodedBytes - accountFor) / 8  + lastBytes;
}

namespace base32
{
    std::optional<QByteArray> decode(const QString &encoded)
    {
        QByteArray result;
        QByteArray bytes = encoded.toLocal8Bit();
        int size = bytes.size(), capacity = size;
        bool ok = false;

        // size should be a multiple of 8 if the input is to be valid base32
        // (smaller data should be padded correctly)
        if (size % 8) {
            return std::nullopt;
        }

        while (size > 0 && bytes[size -1] == '=') {
            size --;
        }

        // based on the amount of padding, determine the exact size of the encoded data
        switch (capacity - size) {
        case 0:
            capacity = determineCapacity(size, 0, 0);
            break;
        case 1:
            capacity = determineCapacity(size, 7, 4);
            break;
        case 3:
            capacity = determineCapacity(size, 5, 3);
            break;
        case 4:
            capacity = determineCapacity(size, 4, 2);
            break;
        case 6:
            capacity = determineCapacity(size, 2, 1);
            break;
        default:
            // invalid amount of padding, reject the input
            return std::nullopt;
        }

        /*
        * We want to fill this buffer *exactly* if possible, to avoid accidental copies of (partial) secrets
        * when filling in the decoded secret later
        */
        result.reserve(capacity);

        char * output = nullptr;
        size_t reportedCapacity = (size_t) capacity;

        int status = oath_base32_decode(bytes.data(), (size_t) size, &output, &reportedCapacity);

        /*
        * sanity check that:
        *  - decoding base32 succeeded
        *  - the library agrees on how big the output buffer should be, i.e. that the preceding allocation logic was correct
        */
        ok = status == OATH_OK && reportedCapacity == ((size_t) capacity);

        /*
        * Avoid += because then strlen() is used which:
        *  - Might branch on unitialised memory according to Valgrind
        *  - Does not work well with embedded \0, which might be used and *is* valid
        */
        if (ok) {
            result.append(output, reportedCapacity);
        }

        /*
        * At this point we have an extra copy of the (decoded) secret in memory.
        * Wipe it and free up the memory.
        *
        * Note the +1 for trailing \0: not strictly necessary but good to be aware?
        */
        if (output) {
            memset(output, '\0', reportedCapacity + 1);
            free(output);
        }

        std::optional<QByteArray> r = std::optional<QByteArray>(result);

        return ok ? r : std::nullopt;
    }
}
