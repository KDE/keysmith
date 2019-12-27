/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "validation.h"

namespace accounts
{

    bool checkId(const QUuid &id)
    {
        return !id.isNull();
    }

    bool checkSecret(const QString &secret)
    {
        return !secret.isEmpty();
    }

    bool checkName(const QString &name)
    {
        return !name.isEmpty();
    }

    bool checkTokenLength(int tokenLength)
    {
        return tokenLength >= 6 && tokenLength <= 8;
    }

    bool checkTimeStep(uint timeStep)
    {
        return timeStep > 0;
    }

    bool checkHotp(const QString &secret, const int tokenLength)
    {
        return checkSecret(secret) && checkTokenLength(tokenLength);
    }

    bool checkHotpAccount(const QUuid &id, const QString &name, const QString &secret, const int tokenLength)
    {
        return checkId(id) && checkName(name) && checkHotp(secret, tokenLength);
    }

    bool checkTotp(const QString &secret, const int tokenLength, const uint timeStep)
    {
        return checkSecret(secret) && checkTokenLength(tokenLength) && checkTimeStep(timeStep);
    }

    bool checkTotpAccount(const QUuid &id, const QString &name, const QString &secret, const int tokenLength, const uint timeStep)
    {
        return checkId(id) && checkName(name) && checkTotp(secret, tokenLength, timeStep);
    }
}
