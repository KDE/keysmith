/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef BASE32_H
#define BASE32_H

#include <QByteArray>
#include <QString>

#include <optional>

namespace base32
{
std::optional<size_t> validate(const QString &encoded, int from = 0, int until = -1);
std::optional<size_t> decode(const QString &encoded, char *const out, size_t outlen, int from = 0, int until = -1);

std::optional<QByteArray> decode(const QString &input);
}
#endif
