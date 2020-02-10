#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(secret_SRCS secrets.cpp)

add_library(secrets_lib STATIC ${secret_SRCS})
target_link_libraries(secrets_lib Qt5::Core sodium base32_lib)
