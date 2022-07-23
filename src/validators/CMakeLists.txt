#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(validator_SRCS
    countervalidator.cpp
    datetimevalidator.cpp
    namevalidator.cpp
    issuervalidator.cpp
    secretvalidator.cpp
    util.cpp
)

add_library(validator_lib STATIC ${validator_SRCS})
target_link_libraries(validator_lib Qt::Core Qt::Gui base32_lib)
