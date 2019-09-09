
enable_testing()
include_directories(BEFORE ../src)

set(Base32_UUT_SRCS ../src/base32.cpp)
set(Test_DEP_LIBS Qt5::Core Qt5::Test)

ecm_add_test(base32-decode.cpp ${Base32_UUT_SRCS} LINK_LIBRARIES ${Test_DEP_LIBS} ${LIBOATH_LIBRARIES} TEST_NAME base32-decode)
