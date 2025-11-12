#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

ecm_test_set_dir_properties(PREFIX_NAME "jobs")

set(Test_DEP_LIBS Qt::Core Qt::Test account_lib account_test_lib test_lib)

set(dispatcher_test_SRCS
    dispatcher-dispatch.cpp
    dispatcher-next.cpp
)

ecm_add_tests(
    ${dispatcher_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
)
