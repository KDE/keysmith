#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Concurrent Qt5::Test keysmith_lib test_lib)

set(app_test_SRCS
    commandline-account-job.cpp
    commandline-options.cpp
)

ecm_add_tests(${app_test_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} NAME_PREFIX app-)
