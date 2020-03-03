/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "util.h"

#include <QRegularExpression>

QRegularExpression spaces_pattern(void)
{
    static const QRegularExpression re("\\s*");
    re.optimize();
    return re;
}

namespace validators
{

    QString strip_spaces(QString &input)
    {
        static const QRegularExpression re = spaces_pattern();
        return input.replace(re, QLatin1String(""));
    }
}
