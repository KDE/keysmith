/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "job.h"

namespace test
{
    void TestJob::finish(void)
    {
        Q_EMIT finished();
    }

    void TestJob::run(void)
    {
        Q_EMIT running();
    }

    JobSignals::JobSignals(QObject *parent): QObject(parent)
    {
    }
}
