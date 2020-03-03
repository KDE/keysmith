#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test oath_lib)
set(oath_lib_test_SRCS
    luhn-checksum.cpp
    count-timesteps.cpp
    encode-token-defaults.cpp
    hotp-algorithm.cpp
    totp-algorithm.cpp
)

ecm_add_tests(
    ${oath_lib_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX oath-
)
