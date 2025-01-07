/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2024 Badin Nicolas <n1coc4cola@gmail.com>
 */
#ifndef SCANNER_QRCODE_H
#define SCANNER_QRCODE_H

#include <QObject>

class QVideoSink;

namespace Prison
{
class VideoScanner;
}

namespace Scanner
{

class QRCode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString decodedText READ decodedText NOTIFY decodedTextChanged)

public:
    explicit QRCode(QObject *parent = nullptr);
    ~QRCode();

    QString decodedText() const;

    Q_INVOKABLE void setCapture(QVideoSink *sink);

Q_SIGNALS:
    void decodedTextChanged();

private:
    Prison::VideoScanner *m_scanner;
    QString m_decodedText;
};

}

#endif
