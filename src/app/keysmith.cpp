/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "keysmith.h"
#include "../logging_p.h"

#include <QClipboard>
#include <QGuiApplication>

KEYSMITH_LOGGER(logger, ".app")

namespace app
{
    Keysmith::Keysmith(QObject *parent): QObject(parent), m_storage(nullptr)
    {
    }

    Keysmith::~Keysmith()
    {
        qCDebug(logger) << "Tearing down Keysmith application; requesting disposal of account storage";
        if (m_storage) {
            m_storage->dispose();
        }
    }

    void Keysmith::copyToClipboard(const QString &text)
    {
        QClipboard * clipboard = QGuiApplication::clipboard();
        if (!clipboard) {
            qCWarning(logger) << "Unable to copy text: clipboard not available";
            return;
        }

        clipboard->setText(text);
    }

    model::SimpleAccountListModel * Keysmith::accountListModel(void)
    {
        return new model::SimpleAccountListModel(storage(), this);
    }

    model::PasswordRequest * Keysmith::passwordRequest(void)
    {
        return new model::PasswordRequest(storage()->secret(), this);
    }

    accounts::AccountStorage * Keysmith::storage(void)
    {
        if (!m_storage) {
            const accounts::SettingsProvider settings([](const accounts::PersistenceAction &action) -> void
            {
                QSettings data(QStringLiteral("org.kde.keysmith"), QStringLiteral("Keysmith"));
                action(data);
            });
            m_storage = accounts::AccountStorage::open(settings);
        }

        return m_storage;
    }
}
