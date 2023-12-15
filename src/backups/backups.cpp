/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "backups.h"

#include "../logging_p.h"

#include <QtCrypto>

#include <QDataStream>
#include <QRandomGenerator>

#define ANDOTP_ITER_LEN 4
#define ANDOTP_SALT_LEN 12
#define ANDOTP_IV_LEN 12
#define ANDOTP_KEY_LEN 32
#define ANDOTP_TAG_LEN 16
#define ANDOTP_SALT_POS (ANDOTP_SALT_LEN + ANDOTP_ITER_LEN)
#define ANDOTP_IV_POS (ANDOTP_IV_LEN + ANDOTP_SALT_POS)
#define ANDOTP_MIN_BACKUP_ITERATIONS 140000
#define ANDOTP_MAX_BACKUP_ITERATIONS 160000

using namespace Qt::Literals::StringLiterals;

KEYSMITH_LOGGER(logger, ".backups")

namespace backups
{

    AndOTPVault::AndOTPVault(QByteArray data) :
        m_data(data)
    {
    }

    QByteArray AndOTPVault::encrypt(const QByteArray& password)
    {
        QRandomGenerator rng;
        rng.bounded(ANDOTP_MIN_BACKUP_ITERATIONS, ANDOTP_MAX_BACKUP_ITERATIONS + 1);
        quint32 iterationCount = rng.generate();
        QCA::InitializationVector salt(ANDOTP_SALT_LEN);
        QCA::InitializationVector iv(ANDOTP_IV_LEN);
        QCA::PBKDF2 deriv;
        QCA::SymmetricKey key(deriv.makeKey(password, salt, ANDOTP_KEY_LEN, iterationCount));
        QCA::Cipher block("aes128"_L1, QCA::Cipher::GCM, QCA::Cipher::Padding::NoPadding);
        block.setup(QCA::Decode, key, iv);
        QCA::MemoryRegion cipher(block.process(m_data));
        QCA::AuthTag tag(block.tag());

        QByteArray ret;
        QDataStream out(&ret, QIODevice::WriteOnly);
        out.setByteOrder(QDataStream::BigEndian);
        out << iterationCount;
        out.writeRawData(salt.constData(), ANDOTP_SALT_LEN);
        out.writeRawData(iv.constData(), ANDOTP_IV_LEN);
        out.writeRawData(key.constData(), ANDOTP_KEY_LEN);
        out.writeRawData(cipher.constData(), cipher.size());
        out.writeRawData(tag.constData(), ANDOTP_TAG_LEN);

        return ret;
    }

    QByteArray AndOTPVault::decrypt(const QByteArray& password)
    {
        quint32 iterationCount;
        QDataStream in(m_data.left(ANDOTP_ITER_LEN));
        in.setByteOrder(QDataStream::BigEndian);
        in >> iterationCount;
        QCA::InitializationVector salt(m_data.mid(ANDOTP_ITER_LEN, ANDOTP_SALT_LEN));
        QCA::InitializationVector iv(m_data.mid(ANDOTP_SALT_POS, ANDOTP_IV_LEN));
        QCA::AuthTag tag(m_data.right(ANDOTP_TAG_LEN));
        QCA::PBKDF2 deriv;
        QCA::SymmetricKey key(deriv.makeKey(password, salt, ANDOTP_KEY_LEN, iterationCount));
        QCA::Cipher block("aes256"_L1, QCA::Cipher::GCM, QCA::Cipher::Padding::NoPadding);
        block.setup(QCA::Decode, key, iv, tag);
        return block.process(m_data.mid(ANDOTP_IV_POS, m_data.size() - ANDOTP_IV_POS - ANDOTP_TAG_LEN)).toByteArray();
    }
}
