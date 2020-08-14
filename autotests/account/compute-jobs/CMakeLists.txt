#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test account_lib account_test_lib test_lib)

set(compute_jobs_test_SRCS
    compute-hotp.cpp
    compute-totp.cpp
)

ecm_add_tests(
    ${compute_jobs_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX account-jobs-
)
