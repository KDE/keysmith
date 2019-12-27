#
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(account_test_lib_SRCS
    job.cpp
    output.cpp
    spy.cpp
)

add_library(account_test_lib STATIC ${account_test_lib_SRCS})
target_link_libraries(account_test_lib Qt5::Core Qt5::Test)
