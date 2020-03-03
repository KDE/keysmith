#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(keysmith_SRCS
    keysmith.cpp
)

add_library(keysmith_lib STATIC ${keysmith_SRCS})
target_link_libraries(keysmith_lib Qt5::Core Qt5::Qml Qt5::Gui model_lib account_lib)
