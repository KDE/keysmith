/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "datetimevalidator.h"
#include "util.h"

namespace validators
{
    std::optional<QDateTime> parseDateTime(const QString &input)
    {
        QDateTime result;

        result = QDateTime::fromString(input, Qt::ISODateWithMs);
        if (result.isValid()) {
            return std::optional<QDateTime>(result);
        }

        result = QDateTime::fromString(input, Qt::ISODate);
        if (result.isValid()) {
            return std::optional<QDateTime>(result);
        }

        return std::nullopt;
    }

    EpochValidator::EpochValidator(const std::function<qint64(void)> clock, QObject *parent) :
        QValidator(parent), m_clock(clock)
    {
    }

    void EpochValidator::fixup(QString &input) const
    {
        input = validators::simplify_spaces(input);
    }

    QValidator::State EpochValidator::validate(QString &input, int &pos) const
    {
        Q_UNUSED(pos);

        const auto parsed = parseDateTime(input);
        if (!parsed) {
            return QValidator::Intermediate;
        }

        return parsed->toMSecsSinceEpoch() <= m_clock() ? QValidator::Acceptable : QValidator::Invalid;
    }
}

#include "moc_datetimevalidator.cpp"
