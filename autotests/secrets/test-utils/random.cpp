/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "random.h"

#include <cstring>

namespace test
{
bool fakeRandom(void *buf, size_t size)
{
    std::memset(buf, 'A', size);
    return true;
}
}
