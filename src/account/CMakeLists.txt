#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(account_SRCS
    account.cpp
    account_p.cpp
    actions_p.cpp
    keys.cpp
    validation.cpp
)

add_library(account_lib STATIC ${account_SRCS})
target_link_libraries(account_lib Qt::Core base32_lib oath_lib secrets_lib)
