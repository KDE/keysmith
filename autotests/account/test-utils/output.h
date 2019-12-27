/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_TEST_UTIL_PATH_H
#define ACCOUNTS_TEST_UTIL_PATH_H

#include <QString>

namespace test
{
    bool ensureOutputDirectory(void);
    bool ensureWritable(const QString &outputRelPath);
    bool copyResource(const QString &resource, const QString &outputRelPath);
    bool copyResourceAsWritable(const QString &resource, const QString &outputRelPath);

    QString slurp(const QString &path);

    QString path(const QString &relPath);
}

#endif
