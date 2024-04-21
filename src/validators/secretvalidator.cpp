/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "secretvalidator.h"

#include "../base32/base32.h"
#include "util.h"

#include <QRegularExpression>
#include <QString>

static QValidator::State check_padding(int length)
{
    if (length == 0) {
        return QValidator::Invalid;
    }

    switch (length % 8) {
    case 1:
    case 3:
    case 6:
        return QValidator::Intermediate;
    default:
        return QValidator::Acceptable;
    }
}

static const QRegularExpression &match_pattern(void)
{
    static const QRegularExpression re(QLatin1String("^[A-Za-z2-7]+=*$"));
    re.optimize();
    return re;
}

namespace validators
{
Base32Validator::Base32Validator(QObject *parent)
    : QValidator(parent)
    , m_pattern(match_pattern())
{
}

void Base32Validator::fixup(QString &input) const
{
    input = validators::strip_spaces(input);
    input = input.toUpper();
    m_pattern.fixup(input);

    int length = input.endsWith(QLatin1Char('=')) ? input.indexOf(QLatin1Char('=')) : input.size();

    QString fixed = input.left(length);
    QValidator::State result = check_padding(length);

    if (result == QValidator::Acceptable) {
        QString maybeFixed = fixed;
        while ((maybeFixed.size() % 8) != 0) {
            maybeFixed += QLatin1Char('=');
        }

        if (base32::validate(maybeFixed)) {
            fixed = maybeFixed;
        }
    }

    input = fixed;
}

QValidator::State Base32Validator::validate(QString &input, int &cursor) const
{
    QValidator::State s = m_pattern.validate(input, cursor);
    if (s == QValidator::Acceptable) {
        // if the basics are covered, check the padding & alignment
        return base32::validate(input) ? QValidator::Acceptable : QValidator::Intermediate;
    } else {
        return s;
    }
}
}

#include "moc_secretvalidator.cpp"
