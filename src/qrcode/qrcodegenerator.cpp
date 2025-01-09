/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Nicolas Badin <n1coc4cola@gmail.com>
 */
#include "qrcodegenerator.h"

#include <QBuffer>
#include <QIcon>

#include <Prison/Barcode>
#include <Prison/Format>

QRCodeGenerator::QRCodeGenerator(QObject *parent)
    : QObject(parent)
{
}

QString QRCodeGenerator::uri() const
{
    QString text = QStringLiteral("otpauth://");
    text += m_isHOtp ? QStringLiteral("hotp/") : QStringLiteral("totp/");

    if (!m_issuer.isEmpty()) {
        text += m_issuer;
        text += QStringLiteral(":");
    }
    text += m_account;

    text += QStringLiteral("?secret=");
    text += m_secret;

    if (!m_issuer.isEmpty()) {
        text += QStringLiteral("&issuer=");
        text += m_issuer;
    }
    if (m_isHOtp) {
        text += QStringLiteral("&counter=");
        text += QString::number(m_counter);
    }

    text += QStringLiteral("&digits=");
    text += QString::number(m_digits);
    text += QStringLiteral("&algorithm=");
    text += m_algorithm;
    text += QStringLiteral("&period=");
    text += QString::number(m_period);

    return QString::fromUtf8(QUrl::toPercentEncoding(text));
}

QUrl imageToUrl(const QImage &image)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");

    QString base64 = QString::fromUtf8(byteArray.toBase64());
    base64.prepend(QStringLiteral("data:image/png;base64,"));

    return QUrl(base64);
}

QUrl QRCodeGenerator::barcode() const
{
    auto gen = Prison::Barcode::create(Prison::BarcodeType::QRCode);
    if (!gen.has_value()) {
        return imageToUrl(QIcon::fromTheme(QStringLiteral("image-missing-symbolic")).pixmap(m_size).toImage());
    }

    gen->setData(uri());
    return imageToUrl(gen->toImage(m_size));
}

void QRCodeGenerator::setIsHOtp(bool b)
{
    if (m_isHOtp != b) {
        m_isHOtp = b;
        Q_EMIT uriChanged();
    }
}

void QRCodeGenerator::setIssuer(const QString &i)
{
    if (m_issuer != i) {
        m_issuer = i;
        Q_EMIT uriChanged();
    }
}

void QRCodeGenerator::setAccount(const QString &a)
{
    if (m_account != a) {
        m_account = a;
        Q_EMIT uriChanged();
    }
}

void QRCodeGenerator::setSecret(const QString &s)
{
    if (m_secret != s) {
        m_secret = s;
        Q_EMIT uriChanged();
    }
}

void QRCodeGenerator::setAlgorithm(const QString &a)
{
    if (m_algorithm != a) {
        m_algorithm = a;
        Q_EMIT uriChanged();
    }
}

void QRCodeGenerator::setDigits(int d)
{
    if (m_digits != d) {
        m_digits = d;
        Q_EMIT uriChanged();
    }
}

void QRCodeGenerator::setPeriod(int p)
{
    if (m_period != p) {
        m_period = p;
        Q_EMIT uriChanged();
    }
}

void QRCodeGenerator::setCounter(int c)
{
    if (m_counter != c) {
        m_counter = c;
        Q_EMIT uriChanged();
    }
}
