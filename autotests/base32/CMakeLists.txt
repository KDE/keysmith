
set(Test_DEP_LIBS Qt5::Core Qt5::Test ${LIBOATH_LIBRARIES} base32_lib)
set(base32_lib_test_SRCS base32-decode.cpp)

ecm_add_tests(
    ${base32_lib_test_SRCS}
    LINK_LIBRARIES ${Test_DEP_LIBS}
    NAME_PREFIX base32-
)
