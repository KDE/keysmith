/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef OATH_UTIL_H
#define OATH_UTIL_H

#include <QCryptographicHash>

#include <optional>

namespace oath
{
    uint luhnChecksum(quint32 value, uint digits);

    std::optional<uint> outputSize(QCryptographicHash::Algorithm algorithm);
}

#endif
