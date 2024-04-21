/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_VALIDATION_H
#define ACCOUNTS_VALIDATION_H

#include "account.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QString>
#include <QUuid>

#include <functional>
#include <optional>

namespace accounts
{
bool checkId(const QUuid &id);
bool checkSecret(const QString &secret);
bool checkName(const QString &name);
bool checkIssuer(const QString &issuer);
bool checkTokenLength(uint tokenLength);
bool checkTimeStep(uint timeStep);
bool checkEpoch(const QDateTime &epoch, const std::function<qint64(void)> &clock = &QDateTime::currentMSecsSinceEpoch);
bool checkOffset(const std::optional<uint> &offset, QCryptographicHash::Algorithm algorithm);
}

#endif
