include_directories(BEFORE ../src)

add_subdirectory(base32)
add_subdirectory(account)
add_subdirectory(validators)

set(Account_UUT_SRCS ../src/account.cpp)
set(Test_DEP_LIBS Qt5::Core Qt5::Test ${LIBOATH_LIBRARIES} base32_lib)

ecm_add_test(hotp-generator-samples.cpp ${Account_UUT_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME hotp-generator-samples)
ecm_add_test(totp-generator-samples.cpp ${Account_UUT_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} TEST_NAME totp-generator-samples)
