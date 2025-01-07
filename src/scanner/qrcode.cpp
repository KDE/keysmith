#include "qrcode.h"

#include <QCamera>
#include <QMediaCaptureSession>

#include <Prison/VideoScanner>

namespace Scanner
{

QRCode::QRCode(QObject *parent)
    : QObject(parent)
    , m_scanner(new Prison::VideoScanner(this))
{
    m_scanner->setFormats(Prison::Format::QRCode);

    connect(m_scanner, &Prison::VideoScanner::resultChanged, [this](const Prison::ScanResult &result) {
        if (!result.hasContent() || !result.hasText()) {
            return;
        }

        m_decodedText = result.content().toString();
        Q_EMIT decodedTextChanged();
    });
}

QRCode::~QRCode()
{
}

QString QRCode::decodedText() const
{
    return m_decodedText;
}

void QRCode::setCapture(QVideoSink *sink)
{
    const auto s = m_scanner->videoSink();

    if (s == sink) {
        return;
    }

    m_scanner->setVideoSink(sink);
}

}
