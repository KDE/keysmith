#
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(validator_SRCS
    countervalidator.cpp
    namevalidator.cpp
    secretvalidator.cpp
    util.cpp
)

add_library(validator_lib STATIC ${validator_SRCS})
target_link_libraries(validator_lib Qt5::Core Qt5::Gui base32_lib)
