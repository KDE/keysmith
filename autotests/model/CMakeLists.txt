#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt::Core Qt::Test model_lib)

set(model_test_SRCS
    milliseconds-left-for-token.cpp
    qr-input.cpp
)

ecm_add_tests(${model_test_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} NAME_PREFIX model-)
