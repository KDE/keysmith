/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

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
             * Fall back to parsing according to C locale for better UX.
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
        Q_UNUSED(cursor);
        // Avoid a hard error on empty string.
        if (input.size() == 0) {
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
