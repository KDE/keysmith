/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "namevalidator.h"

#include <QRegularExpression>
#include <QString>

static const QRegularExpression& match_pattern(void)
{
    static const QRegularExpression re(QLatin1String("^\\S+( \\S+)*$"));
    re.optimize();
    return re;
}

namespace validators
{
    NameValidator::NameValidator(QObject *parent):
        QValidator(parent),
        m_pattern(match_pattern())
    {
    }

    void NameValidator::fixup(QString &input) const
    {
        QString fixed = input.simplified();

        // make sure the user can type in at least one space
        if (input.endsWith(QLatin1Char(' ')) && fixed.size() > 0) {
            fixed += QLatin1Char(' ');
        }

        input = fixed;
    }

    QValidator::State NameValidator::validate(QString &input, int &cursor) const
    {
        return m_pattern.validate(input, cursor);
    }
}
