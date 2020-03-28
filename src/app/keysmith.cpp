/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "keysmith.h"

#include <QClipboard>
#include <QGuiApplication>

namespace app
{
    Keysmith::Keysmith(QObject *parent): QObject(parent), m_storage(nullptr)
    {
    }

    Keysmith::~Keysmith()
    {
        //TODO: log about this
        if (m_storage) {
            m_storage->dispose();
        }
    }

    void Keysmith::copyToClipboard(const QString &text)
    {
        QClipboard * clipboard = QGuiApplication::clipboard();
        if (clipboard) {
            clipboard->setText(text);
        }
        // TODO warn about this
    }

    model::SimpleAccountListModel * Keysmith::accountListModel(void)
    {
        return new model::SimpleAccountListModel(storage(), this);
    }

    accounts::AccountStorage * Keysmith::storage(void)
    {
        if (!m_storage) {
            const accounts::SettingsProvider settings([](const accounts::PersistenceAction &action) -> void
            {
                QSettings data("org.kde.keysmith", "Keysmith");
                action(data);
            });
            m_storage = accounts::AccountStorage::open(settings);
        }

        return m_storage;
    }
}
