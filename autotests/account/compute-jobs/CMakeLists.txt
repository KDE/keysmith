#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

ecm_test_set_dir_properties(PREFIX_NAME "jobs")

set(Test_DEP_LIBS Qt::Core Qt::Test account_lib account_test_lib test_lib)

set(compute_jobs_test_SRCS
    compute-hotp.cpp
    compute-totp.cpp
)

ecm_add_tests(
    ${compute_jobs_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
)
