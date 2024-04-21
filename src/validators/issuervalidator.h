/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#ifndef ISSUER_VALIDATOR_H
#define ISSUER_VALIDATOR_H

#include <QRegularExpressionValidator>
#include <QValidator>

namespace validators
{
class IssuerValidator : public QValidator
{
    Q_OBJECT
public:
    explicit IssuerValidator(QObject *parent = nullptr);
    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &input) const override;

private:
    const QRegularExpressionValidator m_pattern;
};
}

#endif
