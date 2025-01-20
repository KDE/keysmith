#include "otpuri.h"

#include <QStringLiteral>
#include <QUrl>
#include <QUrlQuery>

OtpUri::OtpUri(QObject *parent)
    : QObject(parent)
{
}

OtpUri::OtpUri(const QString &uri, QObject *parent)
    : QObject(parent)
{
    setUri(uri);
}

void OtpUri::setUri(const QString &uri)
{
    m_valid = true;
    m_uri = uri;
    QUrl url(uri);

    if (url.scheme() != QStringLiteral("otpauth")) {
        m_valid = false;
    }

    m_isHOtp = false;

    if (url.host() == QStringLiteral("hotp")) {
        m_isHOtp = true;
    } else if (url.host() != QStringLiteral("totp")) {
        m_valid = false;
    }

    const QUrlQuery query(url.query());
    const QStringList label = url.path().sliced(1).split(QStringLiteral(":"));
    const QString digits = query.queryItemValue(QStringLiteral("digits"));
    const QString period = query.queryItemValue(QStringLiteral("period"));
    const QString counter = query.queryItemValue(QStringLiteral("counter"));
    const QUrl image(query.queryItemValue(QStringLiteral("image")));
    const QUrl website(query.queryItemValue(QStringLiteral("website")));

    m_algorithm = query.queryItemValue(QStringLiteral("algorithm"));
    m_issuer = query.queryItemValue(QStringLiteral("issuer"));
    m_secret = query.queryItemValue(QStringLiteral("secret"));
    m_digits = digits.toInt();
    m_period = period.toInt();
    m_account = label.last();
    m_counter = counter.toInt();

    if (label.length() > 2 || label.isEmpty()) {
        m_valid = false;
    }

    if (m_isHOtp) {
        // We must have the counter.
        // Counter is required and must be put only for HOTP.
        if (counter.isEmpty() || m_counter < 0) {
            m_counter = 0;
            m_valid = false;
        }
    }

    if (m_secret.isEmpty()) {
        m_valid = false;
    }

    if (!m_algorithm.isEmpty()) {
        bool notFound = true;
        const QList<QString> l = {
            QStringLiteral("SHA1"),
            QStringLiteral("SHA256"),
            QStringLiteral("SHA512"),
        };

        for (const auto &e : l) {
            if (e == m_algorithm) {
                notFound = false;
                break;
            }
        }

        if (notFound) {
            m_valid = false;
        }
    } else {
        m_algorithm = QStringLiteral("SHA1");
    }

    if (!digits.isEmpty()) {
        // Spec says from 6 to 8.
        if (m_digits < 6 || m_digits > 8) {
            m_valid = false;
        }
    }

    // Any value is essentially value, but the value must be at least of 1 practically speaking.
    if (!period.isEmpty()) {
        m_period = 30;
    } else if (m_period < 1) {
        m_valid = false;
    }

    // Inconsistency between the issuer named in the prefix (part of the label), and the query's value.
    if (!m_issuer.isEmpty()) {
        if (label.length() == 2 && label[0] != m_issuer) {
            m_valid = false;
        }
    } else if (label.length() == 2) {
        m_issuer = label[0];
    }

    // As these fields are not mandatory, and not official,
    // if there is an issue, just ignore it and skip it.
    if (image.isValid()) {
        m_image = image;
    }
    if (website.isValid()) {
        m_website = website;
    }
}
