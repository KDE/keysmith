/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#ifndef VALIDATOR_UTIL_H
#define VALIDATOR_UTIL_H

#include <QString>

namespace validators
{
QString simplify_spaces(QString &input);
QString strip_spaces(QString &input);
}

#endif
