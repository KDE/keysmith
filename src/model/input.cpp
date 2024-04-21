/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "input.h"

#include <QLocale>

static QDateTime DEFAULT_EPOCH_VALUE = QDateTime::fromMSecsSinceEpoch(0, Qt::UTC);
static QString DEFAULT_EPOCH = DEFAULT_EPOCH_VALUE.toString(Qt::ISODate);
static QString DEFAULT_COUNTER = QLocale::c().toString(0ULL);

namespace model
{

static accounts::Account::Hash toHash(AccountInput::TOTPAlgorithm algorithm)
{
    switch (algorithm) {
    case AccountInput::TOTPAlgorithm::Sha1:
        return accounts::Account::Hash::Sha1;
    case AccountInput::TOTPAlgorithm::Sha256:
        return accounts::Account::Hash::Sha256;
    case AccountInput::TOTPAlgorithm::Sha512:
        return accounts::Account::Hash::Sha512;
    default:
        Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown/unsupported TOTP hashing algorithm?");
        return accounts::Account::Hash::Sha1;
    }
}

AccountInput::AccountInput(QObject *parent)
    : QObject(parent)
    , m_type(TokenType::Totp)
    , m_name(QString())
    , m_issuer(QString())
    , m_secret(QString())
    , m_tokenLength(6U)
    , m_timeStep(30U)
    , m_algorithm(TOTPAlgorithm::Sha1)
    , m_epoch(DEFAULT_EPOCH)
    , m_epochValue(DEFAULT_EPOCH_VALUE)
    , m_checksum(false)
    , m_counter(DEFAULT_COUNTER)
    , m_counterValue(0ULL)
    , m_truncation(std::nullopt)
{
}

void AccountInput::reset(void)
{
    setType(TokenType::Totp);
    setName(QString());
    setIssuer(QString());
    setSecret(QString());
    setTokenLength(6U);
    setTimeStep(30U);
    setAlgorithm(TOTPAlgorithm::Sha1);
    setEpoch(DEFAULT_EPOCH);
    setChecksum(false);
    setCounter(0ULL);
    setDynamicTruncation();
}

void AccountInput::createNewAccount(accounts::AccountStorage *storage) const
{
    if (!storage) {
        Q_ASSERT_X(false, Q_FUNC_INFO, "Storage must be provided");
        return;
    }

    switch (m_type) {
    case Hotp:
        storage->addHotp(m_name, m_issuer, m_secret, m_tokenLength, m_counterValue, m_truncation, m_checksum);
        break;
    case Totp:
        storage->addTotp(m_name, m_issuer, m_secret, m_tokenLength, m_timeStep, m_epochValue, toHash(m_algorithm));
        break;
    default:
        Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown/unsupported token type?");
    }
}

AccountInput::TokenType AccountInput::type(void) const
{
    return m_type;
}

void AccountInput::setType(model::AccountInput::TokenType type)
{
    if (m_type != type) {
        m_type = type;
        Q_EMIT typeChanged();
    }
}

QString AccountInput::name(void) const
{
    return m_name;
}

void AccountInput::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged();
    }
}

QString AccountInput::issuer(void) const
{
    return m_issuer;
}

void AccountInput::setIssuer(const QString &issuer)
{
    if (m_issuer != issuer) {
        m_issuer = issuer;
        Q_EMIT issuerChanged();
    }
}

QString AccountInput::secret(void) const
{
    return m_secret;
}

void AccountInput::setSecret(const QString &secret)
{
    if (m_secret != secret) {
        m_secret = secret;
        Q_EMIT secretChanged();
    }
}

uint AccountInput::tokenLength(void) const
{
    return m_tokenLength;
}

void AccountInput::setTokenLength(uint tokenLength)
{
    if (m_tokenLength != tokenLength) {
        m_tokenLength = tokenLength;
        Q_EMIT tokenLengthChanged();
    }
}

uint AccountInput::timeStep(void) const
{
    return m_timeStep;
}

void AccountInput::setTimeStep(uint timeStep)
{
    if (m_timeStep != timeStep) {
        m_timeStep = timeStep;
        Q_EMIT timeStepChanged();
    }
}

AccountInput::TOTPAlgorithm AccountInput::algorithm(void) const
{
    return m_algorithm;
}

void AccountInput::setAlgorithm(model::AccountInput::TOTPAlgorithm algorithm)
{
    if (m_algorithm != algorithm) {
        m_algorithm = algorithm;
        Q_EMIT algorithmChanged();
    }
}

QString AccountInput::epoch(void) const
{
    return m_epoch;
}

void AccountInput::setEpoch(const QString &epoch)
{
    if (m_epoch != epoch) {
        m_epoch = epoch;
        m_epochValue = validators::parseDateTime(epoch).value_or(DEFAULT_EPOCH_VALUE);
        Q_EMIT epochChanged();
    }
}

bool AccountInput::checksum(void) const
{
    return m_checksum;
}

void AccountInput::setChecksum(bool checksum)
{
    if (m_checksum != checksum) {
        m_checksum = checksum;
        Q_EMIT checksumChanged();
    }
}

QString AccountInput::counter(void) const
{
    return m_counter;
}

void AccountInput::setCounter(const QString &counter, validators::UnsignedLongValidator *validator)
{
    if (!validator) {
        Q_ASSERT_X(false, Q_FUNC_INFO, "Validator must be provided");
        return;
    }

    if (m_counter != counter) {
        m_counter = counter;
        m_counterValue = validators::parseUnsignedInteger(counter, validator->locale()).value_or(0ULL);
        Q_EMIT counterChanged();
    }
}

void AccountInput::setCounter(quint64 counter)
{
    if (m_counterValue != counter) {
        m_counter.setNum(counter);
        m_counterValue = counter;
        Q_EMIT counterChanged();
    }
}

uint AccountInput::truncationOffset(void) const
{
    return m_truncation.value_or(0U);
}

void AccountInput::setTruncationOffset(uint truncationOffset)
{
    if (!m_truncation || *m_truncation != truncationOffset) {
        m_truncation = std::optional<uint>(truncationOffset);
        Q_EMIT truncationChanged();
    }
}

bool AccountInput::fixedTruncation(void) const
{
    return (bool)m_truncation;
}

void AccountInput::setDynamicTruncation(void)
{
    if (m_truncation) {
        m_truncation = std::nullopt;
        Q_EMIT truncationChanged();
    }
}
}

#include "moc_input.cpp"
