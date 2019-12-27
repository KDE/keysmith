/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef APP_KEYSMITH_H
#define APP_KEYSMITH_H

#include "../account/account.h"
#include "../model/accounts.h"

#include <QObject>

namespace app
{
    class Keysmith: public QObject
    {
        Q_OBJECT
    public:
        explicit Keysmith(QObject *parent = nullptr);
        virtual ~Keysmith();
        Q_INVOKABLE model::SimpleAccountListModel * accountListModel(void);
    private:
        accounts::AccountStorage * storage(void);
    private:
        accounts::AccountStorage *m_storage;
    };
}

#endif
