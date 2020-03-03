#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test model_lib)

ecm_add_test(milliseconds-left-for-token.cpp LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME milliseconds-left-for-token NAME_PREFIX model-)
