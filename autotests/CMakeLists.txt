
enable_testing()
include_directories(BEFORE ../src)

add_subdirectory(validators)

set(Base32_UUT_SRCS ../src/base32.cpp)
set(Account_UUT_SRCS ../src/account.cpp ${Base32_UUT_SRCS})
set(Test_DEP_LIBS Qt5::Core Qt5::Test)

ecm_add_test(base32-decode.cpp ${Base32_UUT_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} ${LIBOATH_LIBRARIES} TEST_NAME base32-decode)

ecm_add_test(hotp-generator-samples.cpp ${Account_UUT_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} ${LIBOATH_LIBRARIES} TEST_NAME hotp-generator-samples)
ecm_add_test(totp-generator-samples.cpp ${Account_UUT_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} ${LIBOATH_LIBRARIES} TEST_NAME totp-generator-samples)
