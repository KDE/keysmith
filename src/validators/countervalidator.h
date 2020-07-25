/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#ifndef COUNTER_VALIDATOR_H
#define COUNTER_VALIDATOR_H

#include <QLocale>
#include <QString>
#include <QValidator>

#include <optional>

namespace validators
{
    std::optional<qulonglong> parseUnsignedInteger(const QString &input, const QLocale &locale = QLocale::system());

    class UnsignedLongValidator: public QValidator
    {
        Q_OBJECT
    public:
        explicit UnsignedLongValidator(QObject *parent = nullptr);
        QValidator::State validate(QString &input, int &pos) const override;
        void fixup(QString &input) const override;
    };
}

#endif
