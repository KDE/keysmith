/*****************************************************************************
 * Copyright: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>                  *
 *                                                                           *
 * This project is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This project is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#include "secretvalidator.h"

#include "util.h"

#include <QRegularExpression>
#include <QString>

static QValidator::State check_padding(int length)
{
    if (length == 0) {
        return QValidator::Invalid;
    }

    switch (length % 8)
    {
    case 1:
    case 3:
    case 6:
        return QValidator::Intermediate;
    default:
        return QValidator::Acceptable;
    }
}

static const QRegularExpression& match_pattern(void)
{
    static const QRegularExpression re(QLatin1String("^[A-Za-z2-7]+=*$"));
    re.optimize();
    return re;
}

namespace validators
{
    Base32Validator::Base32Validator(QObject *parent):
        QValidator(parent),
        m_pattern(match_pattern())
    {
    }

    void Base32Validator::fixup(QString &input) const
    {
        input = validators::strip_spaces(input);
        input = input.toUpper();
        m_pattern.fixup(input);

        int length = input.endsWith(QLatin1Char('='))
            ? input.indexOf(QLatin1Char('='))
            : input.size();

        QString fixed = input.left(length);
        QValidator::State result = check_padding(length);

        if (result == QValidator::Acceptable) {
            while ((fixed.size() % 8) != 0) {
                fixed += QLatin1Char('=');
            }
        }

        input = fixed;
    }

    QValidator::State Base32Validator::validate(QString &input, int &cursor) const
    {
        input = strip_spaces(input);

        // spaces may have been removed, adjust cursor
        int size = input.size();
        if (cursor > size) {
            cursor = size;
        }

        // if the basics are covered, check the padding & alignment
        QValidator::State s = m_pattern.validate(input, cursor);
        if (s == QValidator::Acceptable) {
            fixup(input);

            // fixup might have trimmed the string, adjust cursor
            size = input.size();
            if (cursor > size) {
                cursor = size;
            }

            // check if despite auto-fixup the string is still misaligned
            return (size % 8) == 0
                ? QValidator::Acceptable
                : QValidator::Intermediate;
        } else {
            return s;
        }
    }
}
