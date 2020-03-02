/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_VALIDATION_H
#define ACCOUNTS_VALIDATION_H

#include <QString>
#include <QUuid>

namespace accounts
{
    bool checkId(const QUuid &id);
    bool checkSecret(const QString &secret);
    bool checkName(const QString &name);
    bool checkTokenLength(int tokenLength);
    bool checkTimeStep(uint timeStep);
}

#endif
