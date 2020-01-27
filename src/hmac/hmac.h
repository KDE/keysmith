/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef HMAC_H
#define HMAC_H

#include <QByteArray>
#include <QCryptographicHash>

#include <optional>

namespace hmac
{
    std::optional<int> blockSize(QCryptographicHash::Algorithm algorithm);

    bool validateKeySize(QCryptographicHash::Algorithm algorithm, int ksize, bool requireSaneKeyLength = false);
    std::optional<QByteArray> compute(QCryptographicHash::Algorithm algorithm, char * rawKey, int ksize, const QByteArray &message, bool requireSaneKeyLength = false);
}

#endif
