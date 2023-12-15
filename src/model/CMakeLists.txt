#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(model_SRCS
    accounts.cpp
    password.cpp
    input.cpp
    qr.cpp
)

add_library(model_lib STATIC ${model_SRCS})
target_link_libraries(model_lib Qt::Core Qt::Qml Qt::Gui account_lib backups_lib validator_lib uri_lib)
