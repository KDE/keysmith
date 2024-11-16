/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef BACKUPS_H
#define BACKUPS_H

#include <QByteArray>

namespace backups
{
namespace AndOTPVault
{
QByteArray decrypt(QByteArrayView data, QByteArrayView password);
}
}

#endif
