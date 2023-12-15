/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef BACKUPS_H
#define BACKUPS_H

#include <QByteArray>

namespace backups
{
    class AndOTPVault
    {
    public:
        AndOTPVault(QByteArray data);
        QByteArray encrypt(const QByteArray& password);
        QByteArray decrypt(const QByteArray& password);
    private:
        QByteArray m_data;
    };
}

#endif
