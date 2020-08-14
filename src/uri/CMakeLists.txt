#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#
set(uri_SRCS
    uri.cpp
)

add_library(uri_lib STATIC ${uri_SRCS})
target_link_libraries(uri_lib Qt5::Core)
