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

#include "countervalidator.h"

#include "util.h"

namespace validators
{
    std::optional<qulonglong> parse(const QString &input, const QLocale &locale)
    {
        bool ok = false;

        auto v = locale.toULongLong(input, &ok);
        if (ok) {
            return std::optional<qulonglong>(v);
        } else {
            /*
             * Fall back to parsing according to C locale for beter UX.
             * This simplifies copy-pasting from websites which may be a bit
             * biased towards the Anglosphere.
             */
            const QLocale c = QLocale::c();
            v = c.toULongLong(input, &ok);
            return ok ? std::optional<qulonglong>(v) : std::nullopt;
        }
    }

    UnsignedLongValidator::UnsignedLongValidator(QObject *parent):
        QValidator(parent)
    {
    }

    void UnsignedLongValidator::fixup(QString &input) const
    {
        /*
         * First of all: simplify the input to get rid of 'fluff' such as
         * digit group separators in partial input. E.g. if the user wants
         * to type the value 123456 then partial input might become "123,4"
         * at some point which would be invalid.
         */
        const QLocale l = locale();
        const QChar c = l.groupSeparator();
        QString fixed = input.replace(c, QLatin1String(""));
        fixed = validators::strip_spaces(fixed);

        /*
         * Normalise to locale's preferred formatting if possible
         * I.e. if valid, re-instate any decorative fluff like digit group
         * separators depending on the locale.
         *
         * This may also involve switching from Latin script (C locale) to
         * whatever script the configured locale uses natively.
         */
        const auto v = parse(fixed, l);
        if (v) {
            fixed = l.toString(v.value());
        }

        input = fixed;
    }

    QValidator::State UnsignedLongValidator::validate(QString &input, int &cursor) const
    {
        int s = input.size();
        fixup(input);

        /*
         * Size might have changed:
         *
         *  - decreased through the removal of whitespace
         *  - increased through the addition of digit group separators
         *
         * Adjust the cursor as necessary, in particular make sure to
         * advance the cursor if the size has increased and the cursor was
         * at the end of the original input string. In this way the user
         * can simply continue typing input digits while enjoying
         * 'auto formatting'.
         */
        int size = input.size();
        if (size != s && (cursor == s || cursor > size)) {
            cursor = size;
        }

        // Avoid a hard error on empty string.
        if (size == 0) {
            return QValidator::Intermediate;
        } else {

            const QLocale l = locale();
            const auto parsed = parse(input, l);

            /*
             * The actual value is a don't care at this point.
             * All that matters is the input can be parsed
             */
            return parsed ? QValidator::Acceptable : QValidator::Invalid;
        }
    }
}
