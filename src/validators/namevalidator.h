/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#ifndef NAME_VALIDATOR_H
#define NAME_VALIDATOR_H

#include <QRegularExpressionValidator>
#include <QValidator>

namespace validators
{
    class NameValidator: public QValidator
    {
        Q_OBJECT
    public:
        explicit NameValidator(QObject *parent = nullptr);
        QValidator::State validate(QString &input, int &pos) const override;
        void fixup(QString &input) const override;
    private:
        const QRegularExpressionValidator m_pattern;
    };
}

#endif
