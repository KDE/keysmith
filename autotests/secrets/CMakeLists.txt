#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#
add_subdirectory(test-utils)

set(Test_DEP_LIBS Qt::Core Qt::Test secrets_lib secrets_test_lib)
set(secrets_lib_test_SRCS
    encrypt-decrypt-rt.cpp
    key-derivation.cpp
)

ecm_add_tests(
    ${secrets_lib_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX secrets-
)
