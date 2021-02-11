/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "keysmith.h"
#include "../logging_p.h"

#include <QClipboard>
#include <QGuiApplication>

KEYSMITH_LOGGER(logger, ".app.keysmith")

namespace app
{

    static QMetaEnum pagesEnum = QMetaEnum::fromType<Navigation::Page>();

    Navigation::Navigation(QQmlEngine *engine) :
        QObject(engine), m_engine(engine)
    {
        Q_ASSERT_X(m_engine, Q_FUNC_INFO, "must have an engine to work with");
    }

    QString Navigation::name(Navigation::Page page) const
    {
        const char *cname = pagesEnum.valueToKey(page);
        Q_ASSERT_X(cname, Q_FUNC_INFO, "must be able to lookup pages enum constant's name");
        QString result;
        result.append(cname);
        return result;
    }

    void Navigation::navigate(Navigation::Page page, QObject *modelToTransfer)
    {
        const QString route = name(page);
        if (modelToTransfer) {
            m_engine->setObjectOwnership(modelToTransfer, QQmlEngine::JavaScriptOwnership);
        }

        qCDebug(logger) << "Requesting switch to route:" << route << "using (view) model:" << modelToTransfer;
        Q_EMIT routed(route, modelToTransfer);
    }

    void Navigation::push(Navigation::Page page, QObject *modelToTransfer)
    {
        const QString route = name(page);
        if (modelToTransfer) {
            m_engine->setObjectOwnership(modelToTransfer, QQmlEngine::JavaScriptOwnership);
        }

        qCDebug(logger) << "Requesting to push route:" << route << "using (view) model:" << modelToTransfer;
        Q_EMIT pushed(route, modelToTransfer);
    }

    Keysmith::Keysmith(Navigation *navigation, QObject *parent): QObject(parent), m_navigation(navigation), m_storage(nullptr)
    {
    }

    Keysmith::~Keysmith()
    {
        qCDebug(logger) << "Tearing down Keysmith application; requesting disposal of account storage";
        if (m_storage) {
            m_storage->dispose();
        }
    }

    Navigation * Keysmith::navigation(void) const
    {
        return m_navigation;
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
