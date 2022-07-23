#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#
set(oath_SRCS
    oath.cpp
)

add_library(oath_lib STATIC ${oath_SRCS})
target_link_libraries(oath_lib Qt::Core hmac_lib)
