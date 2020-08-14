#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test account_lib account_test_lib secrets_test_lib test_lib)
qt5_add_resources(RCC_SOURCES resources/resources.qrc)

ecm_add_test(load-accounts.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME load-accounts NAME_PREFIX account-jobs-)
ecm_add_test(delete-accounts.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME delete-accounts NAME_PREFIX account-jobs-)
ecm_add_test(save-hotp.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME save-hotp NAME_PREFIX account-jobs-)
ecm_add_test(save-totp.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME save-totp NAME_PREFIX account-jobs-)
ecm_add_test(request-account-password.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME request-account-password NAME_PREFIX account-jobs-)
