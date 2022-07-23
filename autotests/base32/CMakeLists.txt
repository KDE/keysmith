#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt::Core Qt::Test base32_lib)
set(base32_lib_test_SRCS
    base32-decode.cpp
    base32-coding-decoding.cpp
    base32-validate.cpp
)

ecm_add_tests(
    ${base32_lib_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX base32-
)
