/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef ACCOUNTS_ACCOUNT_H
#define ACCOUNTS_ACCOUNT_H

#include <QDateTime>
#include <QObject>
#include <QScopedPointer>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QUuid>
#include <QVector>

#include <functional>
#include <optional>

#include "keys.h"

namespace accounts
{
using PersistenceAction = std::function<void(QSettings &)>;
using SettingsProvider = std::function<void(const PersistenceAction &)>;

class AccountPrivate;
class Account : public QObject
{
    Q_OBJECT
public:
    enum Algorithm { Totp, Hotp };
    Q_ENUM(Algorithm)
    enum Hash { Sha1, Sha256, Sha512 };
    Q_ENUM(Hash)
    explicit Account(AccountPrivate *d, QObject *parent = nullptr);
    QString name(void) const;
    QString token(void) const;
    QString issuer(void) const;
    QString secret(void) const;
    quint64 counter(void) const;
    QDateTime epoch(void) const;
    uint timeStep(void) const;
    std::optional<uint> offset(void) const;
    int tokenLength(void) const;
    bool checksum(void) const;
    Hash hash(void) const;
    Algorithm algorithm(void) const;
public Q_SLOTS:
    void remove(void);
    void recompute(void);
    void setCounter(quint64 counter);
    void advanceCounter(quint64 by = 1ULL);
Q_SIGNALS:
    void tokenChanged(const QString &otp);
    void removed(void);
    void updated(void);

private:
    QScopedPointer<AccountPrivate> m_dptr;
    Q_DECLARE_PRIVATE_D(m_dptr, Account)
};

class AccountStoragePrivate;
class AccountStorage : public QObject
{
    Q_OBJECT
public:
    static AccountStorage *open(const SettingsProvider &settings, AccountSecret *secret = nullptr, QObject *parent = nullptr);
    explicit AccountStorage(const SettingsProvider &settings, QThread *thread, AccountSecret *secret = nullptr, QObject *parent = nullptr);
    void removeAll(const QSet<Account *> &accounts) const;
    bool isAccountStillAvailable(const QString &fullName) const;
    bool isAccountStillAvailable(const QString &name, const QString &issuer) const;
    bool contains(const QString &fullName) const;
    bool contains(const QString &name, const QString &issuer) const;
    Account *get(const QString &fullName) const;
    Account *get(const QString &name, const QString &issuer) const;
    AccountSecret *secret(void) const;
    QVector<QString> accounts(void) const;
    void dispose(void);
    void addHotp(const QString &name,
                 const QString &issuer,
                 const QString &secret,
                 uint tokenLength = 6U,
                 quint64 counter = 0ULL,
                 const std::optional<uint> offset = std::nullopt,
                 bool addChecksum = false);
    void addTotp(const QString &name,
                 const QString &issuer,
                 const QString &secret,
                 uint tokenLength = 6U,
                 uint timeStep = 30U,
                 const QDateTime &epoch = QDateTime::fromMSecsSinceEpoch(0),
                 Account::Hash hash = Account::Hash::Sha1);
    void clearError(void);
    bool hasError(void) const;
    bool isLoaded(void) const;
Q_SIGNALS:
    void added(const QString &fullName);
    void removed(const QString &fullName);
    void error(void);
    void loaded(void);
    void disposed(void);
private Q_SLOTS:
    void unlock(void);
    void load(void);
    void accountRemoved(void);
    void handleDisposal(void);
    void handleError(void);
    void handleLoaded(void);
    void handleHotp(const QUuid id,
                    const QString &name,
                    const QString &issuer,
                    const QByteArray &secret,
                    const QByteArray &nonce,
                    uint tokenLength,
                    quint64 counter,
                    bool fixedTruncation,
                    uint offset,
                    bool checksum);
    void handleTotp(const QUuid id,
                    const QString &name,
                    const QString &issuer,
                    const QByteArray &secret,
                    const QByteArray &nonce,
                    uint tokenLength,
                    uint timeStep,
                    const QDateTime &epoch,
                    Account::Hash hash);

private:
    QScopedPointer<AccountStoragePrivate> m_dptr;
    Q_DECLARE_PRIVATE_D(m_dptr, AccountStorage)
};
}

#endif
