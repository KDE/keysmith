#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(model_SRCS
    accounts.cpp
    password.cpp
)

add_library(model_lib STATIC ${model_SRCS})
target_link_libraries(model_lib Qt5::Core Qt5::Qml Qt5::Gui account_lib validator_lib)
