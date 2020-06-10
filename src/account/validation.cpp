/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "validation.h"

#include "../base32/base32.h"

namespace accounts
{

    bool checkId(const QUuid &id)
    {
        return !id.isNull();
    }

    bool checkSecret(const QString &secret)
    {
        return !secret.isEmpty() && base32::validate(secret);
    }

    bool checkName(const QString &name)
    {
        return !name.isEmpty();
    }

    bool checkIssuer(const QString &issuer)
    {
        return issuer.isNull() || (!issuer.isEmpty() && !issuer.contains(QLatin1Char(':')));
    }

    bool checkTokenLength(int tokenLength)
    {
        return tokenLength >= 6 && tokenLength <= 10;
    }

    bool checkTimeStep(uint timeStep)
    {
        return timeStep > 0;
    }
}
