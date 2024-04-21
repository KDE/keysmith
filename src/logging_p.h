/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef KEYSMITH_LOGGING_PRIVATE_H
#define KEYSMITH_LOGGING_PRIVATE_H

#include <QLoggingCategory>

#define DEFAULT_LOGGER(name, category)                                                                                                                         \
    static const QLoggingCategory &name(void)                                                                                                                  \
    {                                                                                                                                                          \
        static QLoggingCategory c(category, QtMsgType::QtInfoMsg);                                                                                             \
        return c;                                                                                                                                              \
    }

#define KEYSMITH_LOGGER(name, category) DEFAULT_LOGGER(name, "org.kde.keysmith" category)

#endif
