/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "backups.h"

#include "../logging_p.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <QDataStream>
#include <QRandomGenerator>

constexpr auto ANDOTP_ITER_LEN = 4;
constexpr auto ANDOTP_SALT_LEN = 12;
constexpr auto ANDOTP_IV_LEN = 12;
constexpr auto ANDOTP_KEY_LEN = 32;
constexpr auto ANDOTP_TAG_LEN = 16;
constexpr auto ANDOTP_SALT_POS = (ANDOTP_SALT_LEN + ANDOTP_ITER_LEN);
constexpr auto ANDOTP_IV_POS = (ANDOTP_IV_LEN + ANDOTP_SALT_POS);

using namespace Qt::Literals::StringLiterals;

KEYSMITH_LOGGER(logger, ".backups")

namespace backups
{
namespace AndOTPVault
{
QByteArray decrypt(QByteArrayView data, QByteArrayView password)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return {};
    }

    quint32 iterationCount;
    QDataStream in(data.left(ANDOTP_ITER_LEN).toByteArray());
    in.setByteOrder(QDataStream::BigEndian);
    in >> iterationCount;

    const auto salt = data.mid(ANDOTP_ITER_LEN, ANDOTP_SALT_LEN);
    const auto iv = data.mid(ANDOTP_SALT_POS, ANDOTP_IV_LEN);
    const auto ciphertext = data.mid(ANDOTP_IV_POS, data.size() - ANDOTP_IV_POS - ANDOTP_TAG_LEN);
    const auto tag = data.right(ANDOTP_TAG_LEN);

    QByteArray key;
    key.resize(ANDOTP_KEY_LEN);
    if (!PKCS5_PBKDF2_HMAC_SHA1((char *)password.data(),
                                password.size(),
                                (unsigned char *)salt.data(),
                                ANDOTP_SALT_LEN,
                                iterationCount,
                                ANDOTP_KEY_LEN,
                                (unsigned char *)key.data())) {
        qCWarning(logger) << "Key derivation failed.";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) {
        qCWarning(logger) << "decrypt init failed.";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_set_key_length(ctx, key.size());
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr);
    if (!EVP_DecryptInit_ex(ctx, nullptr, nullptr, (const unsigned char *)(key.data()), (const unsigned char *)(iv.data()))) {
        EVP_CIPHER_CTX_free(ctx);
        qCWarning(logger) << "decrypt init failed.";
        return {};
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int resultLength;
    QByteArray outUpdate;
    outUpdate.resize(ciphertext.size() + EVP_CIPHER_CTX_block_size(ctx));

    if (!EVP_DecryptUpdate(ctx, (unsigned char *)outUpdate.data(), &resultLength, (unsigned char *)ciphertext.data(), ciphertext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        qCWarning(logger) << "decrypt update failed.";
        return {};
    }
    outUpdate.resize(resultLength);

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), (unsigned char *)tag.data())) {
        qCWarning(logger) << "decrypt ctrl failed.";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    QByteArray outFinal;
    outFinal.resize(EVP_CIPHER_CTX_block_size(ctx));
    if (!EVP_DecryptFinal_ex(ctx, (unsigned char *)outFinal.data(), &resultLength)) {
        qCWarning(logger) << "decrypt final failed.";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    outFinal.resize(resultLength);

    EVP_CIPHER_CTX_free(ctx);
    return outUpdate + outFinal;
}
}
}
