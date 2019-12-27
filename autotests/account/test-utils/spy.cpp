/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "spy.h"

namespace test
{

    bool signal_eventually_emitted_exactly(QSignalSpy &spy, int times, int timeout)
    {
        return (spy.count() > (times - 1) || spy.wait(timeout)) && spy.count() == times;
    }

    bool signal_eventually_emitted_once(QSignalSpy &spy, int timeout)
    {
        return signal_eventually_emitted_exactly(spy, 1, timeout);
    }

    bool signal_eventually_emitted_twice(QSignalSpy &spy, int timeout)
    {
        return signal_eventually_emitted_exactly(spy, 2, timeout);
    }
}
