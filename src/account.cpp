/*****************************************************************************
 * Copyright: 2013 Michael Zanetti <michael_zanetti@gmx.net>                 *
 *                                                                           *
 * This project is free software: you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This project is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#include "account.h"

#include "base32.h"
#include "oath_p.h"

#include <QDateTime>
#include <QtDebug>

Account::Account(const QUuid &id, QObject *parent) : Account(id, &QDateTime::currentMSecsSinceEpoch, parent) {}

Account::Account(const QUuid &id, const std::function<qint64(void)>& clock, QObject *parent) :
    QObject(parent),
    m_id(id),
    /*
     * Make sure to initialise each member beforehand to some default
     * This is needed because the setters for various properties trigger a re-computation of the OTP token,
     * and that computation depends itself on the values of these fields.
     * I.e. without this the code would branch on uninitialised memory.
     */
    m_type(Account::TypeTOTP),
    m_counter(0),
    m_timeStep(30),
    m_pinLength(6),
    m_clock(clock)
{
    m_totpTimer.setSingleShot(true);
    connect(&m_totpTimer, &QTimer::timeout, this, &Account::generate);
}

QUuid Account::id() const
{
    return m_id;
}

QString Account::name() const
{
    return m_name;
}

void Account::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged();
    }
}

Account::Type Account::type() const
{
    return m_type;
}

void Account::setType(Account::Type type)
{
    if (m_type != type) {
        m_type = type;
//        qDebug() << "setting type" << type;
        Q_EMIT typeChanged();
        generate();
    }
}

QString Account::secret() const
{
    return m_secret;
}

void Account::setSecret(const QString &secret)
{
    if (m_secret != secret) {
        m_secret = secret;
        Q_EMIT secretChanged();
        generate();
    }
}

quint64 Account::counter() const
{
    return m_counter;
}

void Account::setCounter(quint64 counter)
{
    if (m_counter != counter) {
        m_counter = counter;
        Q_EMIT counterChanged();
        generate();
    }
}

int Account::timeStep() const
{
    return m_timeStep;
}

void Account::setTimeStep(int timeStep)
{
    if (m_timeStep != timeStep) {
        m_timeStep = timeStep;
        Q_EMIT timeStepChanged();
        generate();
    }
}

int Account::pinLength() const
{
    return m_pinLength;
}

void Account::setPinLength(int pinLength)
{
    if (m_pinLength != pinLength) {
        m_pinLength = pinLength;
        Q_EMIT pinLengthChanged();
        generate();
    }
}

QString Account::otp() const
{
    return m_otp;
}

qint64 Account::msecsToNext() const
{
    if (m_timeStep <= 0) {
        return 0;
    }
    qint64 now = m_clock();
    qint64 msecsSinceLast = now % (m_timeStep * 1000);
    qint64 msecsToNext = (m_timeStep * 1000) - msecsSinceLast;
    return msecsToNext;
}

void Account::next()
{
    m_counter++;
//    qDebug() << "emitting changed";
    Q_EMIT counterChanged();
    generate();
}

void Account::generate()
{
    if (m_secret.isEmpty()) {
//        qWarning() << "No secret set. Cannot generate otp.";
        return;
    }

    if (m_pinLength <= 0) {
//        qWarning() << "Pin length is" << m_pinLength << ". Cannot generate otp.";
        return;
    }

    if (m_type == TypeTOTP && m_timeStep <= 0) {
//        qWarning() << "Time step is 0. Cannot generate totp";
        return;
    }

//    qDebug() << "generating for account" << m_name;
    std::optional<QByteArray> secret = base32::decode(m_secret);

    if(!secret.has_value()) {
        return;
    }

//    qDebug() << "hexSecret" << hexSecret;
    char code[m_pinLength];
    if (m_type == TypeHOTP) {
        oath_hotp_generate(secret->data(), secret->length(), m_counter, m_pinLength, false, OATH_HOTP_DYNAMIC_TRUNCATION, code);
    } else {
        QDateTime now = QDateTime::fromMSecsSinceEpoch(m_clock());
        oath_totp_generate(secret->data(), secret->length(), now.toTime_t(), m_timeStep, 0, m_pinLength, code);
    }

    m_otp = QLatin1String(code);
//    qDebug() << "Generating secret" << m_name << m_secret << m_counter << m_pinLength << m_otp << m_timeStep;
    Q_EMIT otpChanged();

    if (m_type == TypeTOTP) {

        // QTimer tends to be a wee bit too early...
        // let's just add half a sec to make sure we end up in
        // the current time slot and avoid restarting timers in the ui
        m_totpTimer.setInterval(msecsToNext() + 500);
//        qDebug() << "restarting timer for" << m_name << m_totpTimer.interval() << msecsToNext << QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_totpTimer.start();
    }

}

