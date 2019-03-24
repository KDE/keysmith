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

#include <QDebug>
#include <QDateTime>

#ifndef SIZE_MAX
#define SIZE_MAX UINT_MAX
#endif

extern "C" {
#include <liboath/oath.h>
}

Account::Account(const QUuid &id, QObject *parent) :
    QObject(parent),
    m_id(id),
    m_counter(0),
    m_timeStep(30),
    m_pinLength(6)
{
    m_totpTimer.setSingleShot(true);
    connect(&m_totpTimer, SIGNAL(timeout()), SLOT(generate()));
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
        emit nameChanged();
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
        emit typeChanged();
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
        emit secretChanged();
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
        emit counterChanged();
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
        emit timeStepChanged();
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
        emit pinLengthChanged();
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
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 msecsSinceLast = now % (m_timeStep * 1000);
    qint64 msecsToNext = (m_timeStep * 1000) - msecsSinceLast;
    return msecsToNext;
}

void Account::next()
{
    m_counter++;
//    qDebug() << "emitting changed";
    emit counterChanged();
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
    QByteArray hexSecret = fromBase32(m_secret.toLatin1());
//    qDebug() << "hexSecret" << hexSecret;
    char code[m_pinLength];
    if (m_type == TypeHOTP) {
        oath_hotp_generate(hexSecret.data(), hexSecret.length(), m_counter, m_pinLength, false, OATH_HOTP_DYNAMIC_TRUNCATION, code);
    } else {
        oath_totp_generate(hexSecret.data(), hexSecret.length(), QDateTime::currentDateTime().toTime_t(), m_timeStep, 0, m_pinLength, code);
    }

    m_otp = QLatin1String(code);
//    qDebug() << "Generating secret" << m_name << m_secret << m_counter << m_pinLength << m_otp << m_timeStep;
    emit otpChanged();

    if (m_type == TypeTOTP) {

        // QTimer tends to be a wee bit too early...
        // let's just add half a sec to make sure we end up in
        // the current time slot and avoid restarting timers in the ui
        m_totpTimer.setInterval(msecsToNext() + 500);
//        qDebug() << "restarting timer for" << m_name << m_totpTimer.interval() << msecsToNext << QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_totpTimer.start();
    }

}

QByteArray Account::fromBase32(const QByteArray &input)
{
    int buffer = 0;
    int bitsLeft = 0;
    int count = 0;

    QByteArray result;

    for (int i = 0; i < input.length(); ++i) {

        char ch = input.at(i);

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '-') {
            continue;
        }
        buffer <<= 5;

        if (ch == '0') {
            ch = 'O';
        } else if (ch == '1') {
            ch = 'L';
        } else if (ch == '8') {
            ch = 'B';
        }

        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            ch = (ch & 0x1F) - 1;
        } else if (ch >= '2' && ch <= '7') {
            ch -= '2' - 26;
        } else {
            return QByteArray();
        }

        buffer |= ch;
        bitsLeft += 5;
        if (bitsLeft >= 8) {
            result[count++] = buffer >> (bitsLeft - 8);
            bitsLeft -= 8;
        }

    }

    return result;

}
