#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(Test_DEP_LIBS Qt5::Core Qt5::Test account_lib account_test_lib)
qt5_add_resources(RCC_SOURCES resources/resources.qrc)

ecm_add_test(storage-object-lifecycles.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-object-lifecycles NAME_PREFIX account-)
ecm_add_test(storage-default-lifecycle.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-default-lifecycle NAME_PREFIX account-)
ecm_add_test(storage-aborted-lifecycle.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-aborted-lifecycle NAME_PREFIX account-)
ecm_add_test(hotp-counter-update.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-hotp-counter-update NAME_PREFIX account-)
