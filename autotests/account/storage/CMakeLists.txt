#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

ecm_test_set_dir_properties(PREFIX_NAME_IGNORE)

set(Test_DEP_LIBS Qt::Core Qt::Test account_lib account_test_lib secrets_test_lib test_lib)
qt_add_resources(RCC_SOURCES resources/resources.qrc)

ecm_add_test(storage-object-lifecycles.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-object-lifecycles)
ecm_add_test(storage-default-lifecycle.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-default-lifecycle)
ecm_add_test(storage-aborted-lifecycle.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-aborted-lifecycle)
ecm_add_test(hotp-counter-update.cpp ${RCC_SOURCES} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME storage-hotp-counter-update)
