#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#
set(base32_SRCS base32.cpp)

add_library(base32_lib STATIC ${base32_SRCS})
target_link_libraries(base32_lib Qt5::Core)
