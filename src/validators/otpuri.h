#ifndef OTPURI_H
#define OTPURI_H

#include <QObject>

class OtpUri : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid)
    Q_PROPERTY(int period READ period)
    Q_PROPERTY(int counter READ counter)
    Q_PROPERTY(int digits READ digits)
    Q_PROPERTY(bool isHOtp READ isHOtp)
    Q_PROPERTY(QString uri READ uri WRITE setUri)
    Q_PROPERTY(QString issuer READ issuer)
    Q_PROPERTY(QString account READ account)
    Q_PROPERTY(QString secret READ secret)
    Q_PROPERTY(QString algorithm READ algorithm)

public:
    explicit OtpUri(QObject *parent = nullptr);
    OtpUri(const QString &uri, QObject *parent = nullptr);

    void setUri(const QString &Uri);

    inline bool valid() const
    {
        return m_valid;
    }
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
    inline QString uri() const
    {
        return m_uri;
    }
    inline QString issuer() const
    {
        return m_issuer;
    }
    inline QString account() const
    {
        return m_account;
    }
    inline QString algorithm() const
    {
        return m_algorithm;
    }
    inline QString secret() const
    {
        return m_secret;
    }

private:
    bool m_valid = false;

    QString m_uri;
    bool m_isHOtp;
    QString m_issuer;
    QString m_account;
    QString m_secret;
    QString m_algorithm;
    int m_digits;
    int m_period;
    int m_counter;
};

#endif // OTPURI_H
