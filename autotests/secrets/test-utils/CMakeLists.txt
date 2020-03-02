#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(secrets_test_lib_SRCS
    random.cpp
)

add_library(secrets_test_lib STATIC ${secrets_test_lib_SRCS})
target_link_libraries(secrets_test_lib Qt5::Core Qt5::Test)

