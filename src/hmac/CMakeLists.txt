#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(hmac_SRCS hmac.cpp)

add_library(hmac_lib STATIC ${hmac_SRCS})
target_link_libraries(hmac_lib Qt5::Core)
