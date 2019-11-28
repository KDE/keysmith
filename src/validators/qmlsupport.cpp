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

#include "qmlsupport.h"

#include "countervalidator.h"
#include "namevalidator.h"
#include "secretvalidator.h"

#include <QtQml>

namespace validators
{
    void registerValidatorTypes(void)
    {
        qmlRegisterType<validators::NameValidator>("Oath.Validators", 1, 0, "AccountNameValidator");
        qmlRegisterType<validators::UnsignedLongValidator>("Oath.Validators", 1, 0, "HOTPCounterValidator");
        qmlRegisterType<validators::Base32Validator>("Oath.Validators", 1, 0, "Base32SecretValidator");
    }
}