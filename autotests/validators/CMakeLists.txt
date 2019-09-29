
set(validator_lib_test_SRCS base32-validator.cpp)

ecm_add_tests(
    ${validator_lib_test_SRCS}
    LINK_LIBRARIES Qt5::Core Qt5::Test validator_lib
    NAME_PREFIX validators-
)
