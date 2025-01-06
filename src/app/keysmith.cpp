/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "keysmith.h"
#include "../logging_p.h"

#include "state_p.h"

#include <QClipboard>
#include <QGuiApplication>

KEYSMITH_LOGGER(logger, ".app.keysmith")

namespace app
{

static QMetaEnum pagesEnum = QMetaEnum::fromType<Navigation::Page>();

Navigation::Navigation(QQmlEngine *engine)
    : QObject(engine)
    , m_engine(engine)
{
    Q_ASSERT_X(m_engine, Q_FUNC_INFO, "must have an engine to work with");
}

QString Navigation::name(Navigation::Page page) const
{
    const char *cname = pagesEnum.valueToKey(page);
    Q_ASSERT_X(cname, Q_FUNC_INFO, "must be able to lookup pages enum constant's name");
    QString result;
    result.append(QLatin1String(cname));
    return result;
}

void Navigation::navigate(Navigation::Page page, QObject *modelToTransfer)
{
    const QString route = name(page);
    if (modelToTransfer) {
        m_engine->setObjectOwnership(modelToTransfer, QQmlEngine::JavaScriptOwnership);
    }

    qCDebug(logger) << "Requesting switch to route:" << route << "using (view) model:" << modelToTransfer;
    Q_EMIT routed(page, modelToTransfer);
}

void Navigation::push(Navigation::Page page, QObject *modelToTransfer)
{
    const QString route = name(page);
    if (modelToTransfer) {
        m_engine->setObjectOwnership(modelToTransfer, QQmlEngine::JavaScriptOwnership);
    }

    qCDebug(logger) << "Requesting to push route:" << route << "using (view) model:" << modelToTransfer;
    Q_EMIT pushed(page, modelToTransfer);
}

void Navigation::pop()
{
    Q_EMIT popped();
}

static accounts::AccountStorage *openStorage(void)
{
    qCDebug(logger) << "Initialising Keysmith account storage...";
    const accounts::SettingsProvider settings([](const accounts::PersistenceAction &action) -> void {
        QSettings data(QStringLiteral("org.kde.keysmith"), QStringLiteral("Keysmith"));
        action(data);
    });
    return accounts::AccountStorage::open(settings);
}

Store::Store(void)
    : m_flows(QSharedPointer<FlowState>(new FlowState(), &QObject::deleteLater))
    , m_overview(QSharedPointer<OverviewState>(new OverviewState(), &QObject::deleteLater))
    , m_accounts(QSharedPointer<accounts::AccountStorage>(openStorage(), &accounts::AccountStorage::dispose))
    , m_accountList(QSharedPointer<model::SimpleAccountListModel>(nullptr, &QObject::deleteLater))
{
    m_accountList.reset(new model::SimpleAccountListModel(m_accounts.data()));
}

FlowState *Store::flows(void) const
{
    return m_flows.data();
}

OverviewState *Store::overview(void) const
{
    return m_overview.data();
}

accounts::AccountStorage *Store::accounts(void) const
{
    return m_accounts.data();
}

model::SimpleAccountListModel *Store::accountList(void) const
{
    return m_accountList.data();
}

Keysmith::Keysmith(Navigation *navigation, QObject *parent)
    : QObject(parent)
    , m_store()
    , m_navigation(navigation)
{
}

Keysmith::~Keysmith()
{
    qCDebug(logger) << "Tearing down Keysmith application; disposal of account storage should be requested";
}

Navigation *Keysmith::navigation(void) const
{
    return m_navigation;
}

const Store &Keysmith::store(void) const
{
    return m_store;
};

void Keysmith::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        qCWarning(logger) << "Unable to copy text: clipboard not available";
        return;
    }

    clipboard->setText(text);
}

model::SimpleAccountListModel *Keysmith::accountListModel(void)
{
    return new model::SimpleAccountListModel(m_store.accounts(), this);
}

model::PasswordRequest *Keysmith::passwordRequest(void)
{
    return new model::PasswordRequest(m_store.accounts()->secret(), this);
}
}

#include "moc_keysmith.cpp"
