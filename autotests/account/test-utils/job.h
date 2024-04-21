/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_TEST_UTIL_JOB_H
#define ACCOUNTS_TEST_UTIL_JOB_H

#include "account/actions_p.h"

#include <QObject>

namespace test
{
class TestJob : public accounts::AccountJob
{
    Q_OBJECT
Q_SIGNALS:
    void running(void);
public Q_SLOTS:
    void finish(void);
    void run(void) override;
};

class JobSignals : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void first(void);
    void second(void);
    void quit(void);

public:
    explicit JobSignals(QObject *parent = nullptr);
};
}

#endif
