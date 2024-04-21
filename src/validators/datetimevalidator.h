/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#ifndef DATETIME_VALIDATOR_H
#define DATETIME_VALIDATOR_H

#include <QDateTime>
#include <QLocale>
#include <QValidator>

#include <functional>
#include <optional>

namespace validators
{
std::optional<QDateTime> parseDateTime(const QString &input);

class EpochValidator : public QValidator
{
    Q_OBJECT
public:
    explicit EpochValidator(const std::function<qint64(void)> clock = &QDateTime::currentMSecsSinceEpoch, QObject *parent = nullptr);
    void fixup(QString &input) const override;
    QValidator::State validate(QString &input, int &pos) const override;

private:
    const std::function<qint64(void)> m_clock;
};
}

#endif
