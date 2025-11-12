#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt::Core Qt::Test hmac_lib)

set(hmac_test_SRCS
    hmac-samples.cpp
    hmac-validate-keysize.cpp
)

ecm_add_tests(${hmac_test_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS})
