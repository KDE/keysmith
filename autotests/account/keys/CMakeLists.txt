#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt::Core Qt::Test account_lib account_test_lib secrets_test_lib test_lib)

set(account_secret_test_SRCS
    account-secret-password-flow.cpp
)

ecm_add_tests(
    ${account_secret_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX account-secret-
)
