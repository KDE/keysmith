/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "accounts.h"

#include "../logging_p.h"

KEYSMITH_LOGGER(logger, ".model.accounts")

namespace model
{
    qint64 millisecondsLeftForToken(const QDateTime &epoch, uint timeStep, const std::function<qint64(void)> &clock)
    {
        QDateTime now = QDateTime::fromMSecsSinceEpoch(clock());
        if (!epoch.isValid() || !now.isValid() || timeStep == 0) {
            qCDebug(logger) << "Unable to compute milliseconds left: invalid arguments";
            return -1;
        }

        /*
         * Avoid integer overflow by casting to the wider type first before multiplying.
         * Not likely to happen 'in the wild', but good practice nevertheless
         */
        qint64 step = ((qint64) timeStep) * 1000LL;

        qint64 diff = epoch.msecsTo(now);

        /*
         * Compensate for the fact that % operator is not the same as mathematical mod in case diff is negative.
         * diff is negative when the given epoch is in the 'future' compared to the current clock value.
         */
        return diff < 0 ? - (diff % step) : step - (diff % step);
    }

    AccountView::AccountView(accounts::Account *model, QObject *parent) :
        QObject(parent), m_model(model)
    {
        QObject::connect(model, &accounts::Account::tokenChanged, this, &AccountView::tokenChanged);
        QObject::connect(this, &AccountView::remove, model, &accounts::Account::remove);
        QObject::connect(this, &AccountView::recompute, model, &accounts::Account::recompute);
        QObject::connect(this, &AccountView::advanceCounter, model, &accounts::Account::advanceCounter);
        QObject::connect(this, &AccountView::setCounter, model, &accounts::Account::setCounter);
    }

    bool AccountView::isHotp(void) const
    {
        return m_model->algorithm() == accounts::Account::Hotp;
    }

    bool AccountView::isTotp(void) const
    {
        return m_model->algorithm() == accounts::Account::Totp;
    }

    QString AccountView::name(void) const
    {
        return m_model->name();
    }

    QString AccountView::issuer(void) const
    {
        return m_model->issuer();
    }

    QString AccountView::token(void) const
    {
        return m_model->token();
    }

    quint64 AccountView::counter(void) const
    {
        return m_model->counter();
    }

    uint AccountView::timeStep(void) const
    {
        return m_model->timeStep();
    }

    qint64 AccountView::millisecondsLeftForToken(void) const
    {
        if (!isTotp()) {
            qCDebug(logger) << "Unable to compute milliseconds left for token, wrong account type:" << m_model->algorithm();
            return -1;
        }

        return model::millisecondsLeftForToken(m_model->epoch(), m_model->timeStep());
    }

    SimpleAccountListModel::SimpleAccountListModel(accounts::AccountStorage *storage, QObject *parent) :
        QAbstractListModel(parent), m_storage(storage), m_has_error(false), m_index(QVector<QString>())
    {
        QObject::connect(storage, &accounts::AccountStorage::added, this, &SimpleAccountListModel::added);
        QObject::connect(storage, &accounts::AccountStorage::removed, this, &SimpleAccountListModel::removed);
        QObject::connect(storage, &accounts::AccountStorage::error, this, &SimpleAccountListModel::handleError);
        QObject::connect(storage, &accounts::AccountStorage::loaded, this, &SimpleAccountListModel::loadedChanged);

        beginResetModel();
        const auto accounts = m_storage->accounts();
        for (const QString &name : accounts) {
            populate(name, createView(name));
        }
        m_has_error = storage->hasError();
        endResetModel();
    }


    AccountView * SimpleAccountListModel::createView(const QString &name)
    {
        accounts::Account * existingAccount = m_storage->get(name);
        if (!existingAccount) {
            qCDebug(logger) << "Account storage did not yield a valid account object for account name";
            return nullptr;
        }

        return new AccountView(existingAccount, this);
    }

    void SimpleAccountListModel::populate(const QString &name, AccountView *account)
    {
        if (!account) {
            qCDebug(logger) << "Not populating account without a valid account view object";
            return;
        }

        m_index.append(name);
        m_accounts[name] = account;
        Q_EMIT account->recompute();
    }

    bool SimpleAccountListModel::error(void) const
    {
        return m_has_error;
    }

    void SimpleAccountListModel::setError(bool markAsError)
    {
        if (!markAsError && m_storage->hasError()) {
            m_storage->clearError();
        }

        if (markAsError != m_has_error) {
            m_has_error = markAsError;
            Q_EMIT errorChanged();
        }
    }

    void SimpleAccountListModel::handleError(void)
    {
        setError(true);
    }

    bool SimpleAccountListModel::loaded(void) const
    {
        return m_storage->isLoaded();
    }

    accounts::Account::Hash SimpleAccountListModel::toHash(TOTPAlgorithms value)
    {
        switch (value) {
        case TOTPAlgorithms::Sha1:
            return accounts::Account::Hash::Sha1;
        case TOTPAlgorithms::Sha256:
            return accounts::Account::Hash::Sha256;
        case TOTPAlgorithms::Sha512:
            return accounts::Account::Hash::Sha512;
        default:
            Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown/unsupported totp algorithm value");
            return accounts::Account::Hash::Sha1;
        }
    }

    void SimpleAccountListModel::addAccount(AccountInput *input)
    {
        if (!input) {
            qCDebug(logger) << "Not adding account, no input provided";
            return;
        }
        input->createNewAccount(m_storage);
    }

    QHash<int, QByteArray> SimpleAccountListModel::roleNames(void) const
    {
        QHash<int, QByteArray> roles;
        roles[NonStandardRoles::AccountRole] = "account";
        return roles;
    }

    QVariant SimpleAccountListModel::data(const QModelIndex &account, int role) const
    {
        if (!account.isValid()) {
            qCDebug(logger) << "Not returning any data, model index is invalid";
            return QVariant();
        }

        int accountIndex = account.row();
        if (accountIndex < 0 || m_index.size() < accountIndex) {
            qCDebug(logger) << "Not returning any data, model index is out of bounds:" << accountIndex
                << "model size is:" << m_index.size();
            return QVariant();
        }

        if (role != NonStandardRoles::AccountRole) {
            qCDebug(logger) << "Not returning any data, unknown role:" << role;
            return QVariant();
        }

        const QString accountName = m_index.at(accountIndex);
        auto model = m_accounts.value(accountName, nullptr);
        if (!model) {
            qCDebug(logger) << "Not returning any data, unable to find associated account for:" << accountIndex;
            return QVariant();
        }

        return QVariant::fromValue(model);
    }

    int SimpleAccountListModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return m_index.size();
    }

    void SimpleAccountListModel::added(const QString &account)
    {
        auto newAccount = createView(account);
        if (!newAccount) {
            qCDebug(logger) << "Unable to handle added account: unable to construct account view object";
            return;
        }

        if (m_accounts.contains(account)) {
            qCDebug(logger) << "Added account already/still part of the model: requesting removal of the old one from the model first";
            removed(account);
        }

        int accountIndex = m_index.size();
        qCDebug(logger) << "Adding (new) account to the model at position:" << accountIndex;

        beginInsertRows(QModelIndex(), accountIndex, accountIndex);
        populate(account, newAccount);
        endInsertRows();
    }

    void SimpleAccountListModel::removed(const QString &account)
    {
        int accountIndex = m_index.indexOf(account);
        if (accountIndex < 0) {
            qCDebug(logger) << "Unable to handle account removal: account not part of the model";
            return;
        }

        AccountView *v = nullptr;

        qCDebug(logger) << "Removing (old) account from the model at position:" << accountIndex;
        beginRemoveRows(QModelIndex(), accountIndex, accountIndex);
        m_index.remove(accountIndex);
        v = m_accounts.take(account);
        endRemoveRows();

        if (v) {
            v->deleteLater();
        }
    }

    bool SimpleAccountListModel::isAccountStillAvailable(const QString &name, const QString &issuer) const
    {
        return m_storage && m_storage->isAccountStillAvailable(name, issuer);
    }

    AccountNameValidator::AccountNameValidator(QObject *parent) :
        QValidator(parent), m_validateAvailability(true), m_issuer(std::nullopt), m_accounts(nullptr), m_delegate(nullptr)
    {
    }

    bool AccountNameValidator::validateAvailability(void) const
    {
        return m_validateAvailability;
    }

    void AccountNameValidator::setValidateAvailability(bool enabled)
    {
        if (enabled != m_validateAvailability) {
            m_validateAvailability = enabled;
            Q_EMIT validateAvailabilityChanged();
        }
    }

    QString AccountNameValidator::issuer(void) const
    {
        return m_issuer
            ? *m_issuer
            : QStringLiteral(":: WARNING: dummy invalid issuer; this is meant as a write-only property anyway ::");
    }

    void AccountNameValidator::setIssuer(const QString &issuer)
    {
        if (m_issuer && issuer == *m_issuer) {
            qCDebug(logger) << "Ignoring new issuer: same as the current issuer";
            return;
        }

        m_issuer.emplace(issuer);
        Q_EMIT issuerChanged();
    }

    QValidator::State AccountNameValidator::validate(QString &input, int &pos) const
    {
        QValidator::State result = m_delegate.validate(input, pos);
        if (!m_validateAvailability) {
            qCDebug(logger) << "Not validating account availability: explicitly disabled";
            return result;
        }

        if (!m_accounts) {
            qCDebug(logger) << "Unable to validat account name: missing accounts model object";
            return QValidator::Invalid;
        }

        if (!m_issuer) {
            qCDebug(logger) << "Unable to validate account name: missing issuer";
            return QValidator::Invalid;
        }

        return result != QValidator::Acceptable || m_accounts->isAccountStillAvailable(input, *m_issuer)
            ? result
            : QValidator::Intermediate;
    }

    void AccountNameValidator::fixup(QString &input) const
    {
        m_delegate.fixup(input);
    }

    SimpleAccountListModel * AccountNameValidator::accounts(void) const
    {
        return m_accounts;
    }

    void AccountNameValidator::setAccounts(SimpleAccountListModel *accounts)
    {
        if (!accounts) {
            qCDebug(logger) << "Ignoring new accounts model: not a valid object";
            return;
        }

        m_accounts = accounts;
        Q_EMIT accountsChanged();
    }

    SortedAccountsListModel::SortedAccountsListModel(QObject *parent) : QSortFilterProxyModel(parent)
    {
    }

    void SortedAccountsListModel::setSourceModel(QAbstractItemModel *sourceModel)
    {
        SimpleAccountListModel *model = qobject_cast<SimpleAccountListModel*>(sourceModel);
        if (!model) {
            qCDebug(logger) << "Not setting source model: it is not an accounts list model!";
            return;
        }

        QSortFilterProxyModel::setSourceModel(sourceModel);
        qCDebug(logger) << "Updating properties & resorting the model";
        setSortRole(SimpleAccountListModel::NonStandardRoles::AccountRole);
        setDynamicSortFilter(true);
        setSortLocaleAware(true);
        sort(0);
    }

    bool SortedAccountsListModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
    {
        QAbstractItemModel *source = sourceModel();
        Q_ASSERT_X(source, Q_FUNC_INFO, "should have a source model at this point");

        const SimpleAccountListModel *model = qobject_cast<const SimpleAccountListModel*>(source);
        // useless junk: implement sorting as no-op: claim equality between left & right
        if (!model) {
            qCDebug(logger) << "Short-circuiting lessThan operator: source model is not an accounts list model!";
            return false;
        }

        const QVariant leftValue = model->data(source_left, SimpleAccountListModel::NonStandardRoles::AccountRole);
        const QVariant rightValue = model->data(source_right, SimpleAccountListModel::NonStandardRoles::AccountRole);

        const AccountView * leftAccount = leftValue.isNull() ? nullptr : leftValue.value<AccountView*>();
        const AccountView * rightAccount = rightValue.isNull() ? nullptr : rightValue.value<AccountView*>();

        // useless junk: implement sorting as no-op: claim left == right
        if (!leftAccount && !rightAccount) {
            qCDebug(logger) << "Short-circuiting lessThan operator: both source model indices do not point to accounts";
            return false;
        }

        // Sort actual accounts before useless junk: claim left >= right
        if (!leftAccount) {
            qCDebug(logger) << "Short-circuiting lessThan operator: left source model index does not point to an account";
            return false;
        }

        // Sort actual accounts before useless junk: claim left < right
        if (!rightAccount) {
            qCDebug(logger) << "Short-circuiting lessThan operator: right source model index does not point to an account";
            return true;
        }

        const QString leftIssuer = leftAccount->issuer();
        const QString rightIssuer = rightAccount->issuer();

        // both issuers are null: sort by account name
        if (leftIssuer.isNull() && rightIssuer.isNull()) {
            return leftAccount->name().localeAwareCompare(rightAccount->name()) < 0;
        }

        // Sort accounts without issuer to the top: claim left < right
        if (leftIssuer.isNull()) {
            return true;
        }

        // Sort accounts without issuer to the top: claim left >= right
        if (rightIssuer.isNull()) {
            return false;
        }

        // actual sorting by account issuer, then name
        int issuer = leftIssuer.localeAwareCompare(rightIssuer);
        return issuer == 0 ? leftAccount->name().localeAwareCompare(rightAccount->name()) < 0 : issuer < 0;
    }
}
