#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(account_test_lib_SRCS
    job.cpp
    output.cpp
    secret.cpp
)

add_library(account_test_lib STATIC ${account_test_lib_SRCS})
target_link_libraries(account_test_lib Qt::Core Qt::Test account_lib)
