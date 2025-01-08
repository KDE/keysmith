#ifndef QRCODEIMAGE_H
#define QRCODEIMAGE_H

#include <QImage>
#include <QObject>
#include <QUrl>

namespace Prison
{
class BarcodeScanner;
}

namespace Scanner
{

class QRCodeImage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRect imageRect READ rect WRITE setRect)
    Q_PROPERTY(QImage image READ image WRITE setImage)
    Q_PROPERTY(QUrl source READ source WRITE setSource)
    Q_PROPERTY(QString decodedText READ decodedText NOTIFY decodedTextChanged)
    Q_PROPERTY(QString supportedFileTypes READ supportedImages)

public:
    QRCodeImage(QObject *parent = nullptr);

    static QStringList supportedImageFormats();
    static QString supportedImages();

    inline QImage image() const
    {
        return m_img;
    }
    inline QRect rect() const
    {
        return m_rect;
    }
    inline QString decodedText() const
    {
        return m_decodedText;
    }
    inline QUrl source() const
    {
        return m_src;
    }

    void setImage(const QImage &img);
    void setRect(const QRect &r);
    void setSource(const QUrl &url);

Q_SIGNALS:
    void decodedTextChanged();

private:
    QUrl m_src;
    QImage m_img;
    QRect m_rect;
    QString m_decodedText;
    Prison::BarcodeScanner *m_scanner;
};

}

#endif // QRCODEIMAGE_H
