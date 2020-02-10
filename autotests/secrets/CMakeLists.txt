#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test secrets_lib)
set(secrets_lib_test_SRCS
    encrypt-decrypt-rt.cpp
    key-derivation.cpp
)

ecm_add_tests(
    ${secrets_lib_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX secrets-
)
