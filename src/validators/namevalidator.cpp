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
        fixup(input);

        // spaces may have been removed, adjust cursor
        int size = input.size();
        if (cursor > size) {
            cursor = size;
        }

        return m_pattern.validate(input, cursor);
    }
}
