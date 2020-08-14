#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test account_lib account_test_lib test_lib)

set(dispatcher_test_SRCS
    dispatcher-dispatch.cpp
    dispatcher-next.cpp
)

ecm_add_tests(
    ${dispatcher_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX account-jobs-
)
