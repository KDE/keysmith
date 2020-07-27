/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "issuervalidator.h"
#include "util.h"

#include <QRegularExpression>
#include <QString>

static const QRegularExpression& match_pattern(void)
{
    /*
     * Pattern to check that issuer names:
     *
     *  - do not contain colons
     *  - start and end with a character which is not whitespace
     *  - do not contain any other whitespace besides spaces
     *  - have at most one space between non-whitespace characters
     */
    static const QRegularExpression re(QLatin1String("^[^\\s:]+( [^\\s:]+)*$"));
    re.optimize();
    return re;
}

namespace validators
{
    IssuerValidator::IssuerValidator(QObject *parent):
        QValidator(parent),
        m_pattern(match_pattern())
    {
    }

    void IssuerValidator::fixup(QString &input) const
    {
        input = validators::simplify_spaces(input.remove(QLatin1Char(':')));
    }

    QValidator::State IssuerValidator::validate(QString &input, int &cursor) const
    {
        return input.isEmpty() ? QValidator::Acceptable : m_pattern.validate(input, cursor);
    }
}
