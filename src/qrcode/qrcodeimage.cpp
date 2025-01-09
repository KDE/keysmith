/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Nicolas Badin <n1coc4cola@gmail.com>
 */
#include "qrcodeimage.h"

#include <QFile>
#include <QImageReader>
#include <QtConcurrent/QtConcurrent>

#include <Prison/ImageScanner>
#include <Prison/ScanResult>

namespace Scanner
{

QRCodeImage::QRCodeImage(QObject *parent)
    : QObject(parent)
{
}

QString QRCodeImage::supportedImages()
{
    QString text = tr("Images (");
    // Iterate through supported formats and create file extensions
    for (const QByteArray &format : QImageReader::supportedImageFormats()) {
        text.append(QStringLiteral(" *."));
        text.append(QString::fromLatin1(format));
    }

    text.append(QStringLiteral(")"));
    return text;
}

QStringList QRCodeImage::supportedImageFormats()
{
    QStringList fileExtensions;

    // Iterate through supported formats and create file extensions
    for (const QByteArray &format : QImageReader::supportedImageFormats()) {
        fileExtensions.append(QStringLiteral("*.") + QString::fromLatin1(format));
    }

    return fileExtensions;
}

void QRCodeImage::setImage(const QImage &img)
{
    if (img != m_img) {
        m_img = img;
    }
}

void QRCodeImage::setRect(const QRect &r)
{
    if (r != m_rect) {
        m_rect = r;

        QtConcurrent::run([this, r]() {
            return Prison::ImageScanner::scan(m_img.copy(r), Prison::Format::QRCode);
        }).then([this](const Prison::ScanResult &result) {
            if (!result.hasContent() || !result.hasText()) {
                return;
            }
            m_decodedText = result.text();
            Q_EMIT decodedTextChanged();
        });
    }
}

void QRCodeImage::setSource(const QUrl &src)
{
    if (m_src != src) {
        m_src = src;
        m_img = QImage(src.toLocalFile());

        QtConcurrent::run([this]() {
            return Prison::ImageScanner::scan(m_img, Prison::Format::QRCode);
        }).then([this](const Prison::ScanResult &result) {
            if (!result.hasContent() || !result.hasText()) {
                return;
            }
            m_decodedText = result.text();
            Q_EMIT decodedTextChanged();
        });
    }
}

}
