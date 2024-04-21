/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef SECRETS_TEST_UTIL_RANDOM_H
#define SECRETS_TEST_UTIL_RANDOM_H

#include <stddef.h>

namespace test
{
bool fakeRandom(void *buf, size_t size);
}

#endif
