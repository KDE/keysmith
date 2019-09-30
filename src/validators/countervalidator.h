/*****************************************************************************
 * Copyright: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>                  *
 *                                                                           *
 * This project is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This project is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#ifndef COUNTER_VALIDATOR_H
#define COUNTER_VALIDATOR_H

#include <QLocale>
#include <QString>
#include <QValidator>

#include <optional>

namespace validators
{
    std::optional<qulonglong> parse(const QString &input, const QLocale &locale = QLocale::system());

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
