/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "random.h"

#include <string.h>

namespace test
{
    bool fakeRandom(void *buf, size_t size)
    {
        memset(buf, 'A', size);
        return true;
    }
}
