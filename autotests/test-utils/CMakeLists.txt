#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(test_lib_SRCS
    spy.cpp
)

add_library(test_lib STATIC ${test_lib_SRCS})
target_link_libraries(test_lib Qt5::Core Qt5::Test)

