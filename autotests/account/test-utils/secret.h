/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_TEST_UTIL_ACCOUNT_SECRET_H
#define ACCOUNTS_TEST_UTIL_ACCOUNT_SECRET_H

#include "account/keys.h"
#include "secrets/secrets.h"

#include <QByteArray>
#include <QString>

#include <optional>

namespace test
{
    secrets::SecureMasterKey * useDummyPassword(accounts::AccountSecret *secret);
    secrets::SecureMasterKey * useDummyPassword(accounts::AccountSecret *secret,
                                                QString &password, QByteArray &salt,
                                                const secrets::EncryptedSecret &challenge);

    std::optional<secrets::EncryptedSecret> encrypt(const accounts::AccountSecret *secret,
                                                    const QByteArray &tokenSecret);
}

#endif
