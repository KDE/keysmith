#
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(account_SRCS
    account.cpp
    account_p.cpp
    actions_p.cpp
    validation.cpp
)

add_library(account_lib STATIC ${account_SRCS})
target_link_libraries(account_lib Qt5::Core base32_lib ${LIBOATH_LIBRARIES})
