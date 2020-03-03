#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test account_lib)

set(validation_test_SRCS
    account-validation.cpp
)

ecm_add_tests(
    ${validation_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX account-
)
