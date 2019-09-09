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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QUuid>
#include <QTimer>

#include <functional>

class Account : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString secret READ secret WRITE setSecret NOTIFY secretChanged)
    Q_PROPERTY(quint64 counter READ counter WRITE setCounter NOTIFY counterChanged)
    Q_PROPERTY(int timeStep READ timeStep WRITE setTimeStep NOTIFY timeStepChanged)
    Q_PROPERTY(int pinLength READ pinLength WRITE setPinLength NOTIFY pinLengthChanged)
    Q_PROPERTY(QString otp READ otp NOTIFY otpChanged)
public:
    enum Type {
        TypeHOTP,
        TypeTOTP
    };

    explicit Account(const QUuid &id, QObject *parent = 0);
    explicit Account(const QUuid &id, const std::function<qint64(void)>& clock, QObject *parent = 0);

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

    Type type() const;
    void setType(Type type);

    QString secret() const;
    void setSecret(const QString &secret);

    quint64 counter() const;
    void setCounter(quint64 counter);

    int timeStep() const;
    void setTimeStep(int timeStep);

    int pinLength() const;
    void setPinLength(int pinLength);

    QString otp() const;

    Q_INVOKABLE qint64 msecsToNext() const;

signals:
    void nameChanged();
    void typeChanged();
    void secretChanged();
    void counterChanged();
    void timeStepChanged();
    void pinLengthChanged();
    void otpChanged();

public slots:
    void generate();
    void next();

private:
    QUuid m_id;
    QString m_name;
    Type m_type;
    QString m_secret;
    quint64 m_counter;
    int m_timeStep;
    int m_pinLength;
    QString m_otp;
    QTimer m_totpTimer;
    const std::function<qint64(void)> m_clock;
};

#endif // ACCOUNT_H
