#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

ecm_test_set_dir_properties(PREFIX_NAME "jobs")

set(Test_DEP_LIBS Qt::Core Qt::Test account_lib account_test_lib secrets_test_lib test_lib)
qt_add_resources(RCC_SOURCES resources/resources.qrc)

ecm_add_test(load-accounts.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME load-accounts)
ecm_add_test(delete-accounts.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME delete-accounts)
ecm_add_test(save-hotp.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME save-hotp)
ecm_add_test(save-totp.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME save-totp)
ecm_add_test(request-account-password.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME request-account-password)
