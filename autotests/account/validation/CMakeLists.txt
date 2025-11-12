#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

ecm_test_set_dir_properties(PREFIX_NAME_IGNORE)

set(Test_DEP_LIBS Qt::Core Qt::Test account_lib)

set(validation_test_SRCS
    account-validation.cpp
)

ecm_add_tests(
    ${validation_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
)
