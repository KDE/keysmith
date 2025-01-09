/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Nicolas Badin <n1coc4cola@gmail.com>
 */
#ifndef QRCODEGENERATOR_H
#define QRCODEGENERATOR_H

#include <QImage>
#include <QObject>
#include <QUrl>

class QRCodeGenerator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int period READ period WRITE setPeriod)
    Q_PROPERTY(int counter READ counter WRITE setCounter)
    Q_PROPERTY(int digits READ digits WRITE setDigits)
    Q_PROPERTY(bool isHOtp READ isHOtp WRITE setIsHOtp)
    Q_PROPERTY(QString issuer READ issuer WRITE setIssuer)
    Q_PROPERTY(QString account READ account WRITE setAccount)
    Q_PROPERTY(QString secret READ secret WRITE setSecret)
    Q_PROPERTY(QString algorithm READ algorithm WRITE setAlgorithm)
    Q_PROPERTY(QString uri READ uri NOTIFY uriChanged)
    Q_PROPERTY(QSize size READ size WRITE setSize)

public:
    explicit QRCodeGenerator(QObject *parent = nullptr);

    QString uri() const;
    Q_INVOKABLE QUrl barcode() const;

    inline int period() const
    {
        return m_period;
    }
    inline int counter() const
    {
        return m_counter;
    }
    inline int digits() const
    {
        return m_digits;
    }
    inline bool isHOtp() const
    {
        return m_isHOtp;
    }
    inline QString issuer() const
    {
        return m_issuer;
    }
    inline QString account() const
    {
        return m_account;
    }
    inline QString secret() const
    {
        return m_secret;
    }
    inline QString algorithm() const
    {
        return m_algorithm;
    }
    inline QSize size() const
    {
        return m_size;
    }

    void setIsHOtp(bool);
    void setIssuer(const QString &);
    void setAccount(const QString &);
    void setSecret(const QString &);
    void setAlgorithm(const QString &);
    void setDigits(int);
    void setPeriod(int);
    void setCounter(int);

    inline void setSize(const QSize &s)
    {
        m_size = s;
    }

Q_SIGNALS:
    void uriChanged();

private:
    QSize m_size;
    bool m_isHOtp;
    QString m_issuer;
    QString m_account;
    QString m_secret;
    QString m_algorithm;
    int m_digits;
    int m_period;
    int m_counter;
};

#endif // QRCODEGENERATOR_H
