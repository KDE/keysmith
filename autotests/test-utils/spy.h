/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef TEST_UTIL_SPY_H
#define TEST_UTIL_SPY_H

#include <QSignalSpy>

namespace test
{
bool signal_eventually_emitted_exactly(QSignalSpy &spy, int times, int timeout = 500);
bool signal_eventually_emitted_once(QSignalSpy &spy, int timeout = 500);
bool signal_eventually_emitted_twice(QSignalSpy &spy, int timeout = 500);
}

#endif
