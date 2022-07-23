#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt::Core Qt::Test uri_lib)

set(uri_test_SRCS
    percent-encoding.cpp
    qr-parsing.cpp
)

ecm_add_tests(${uri_test_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} NAME_PREFIX uri-)
