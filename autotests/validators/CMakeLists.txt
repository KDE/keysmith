
set(validator_lib_test_SRCS
    base32-validator.cpp
)

#
# Using macros to take care of boilerplate test setup confuses Automoc a bit
# It still works without registering the macro, but this way the mistaken
# warnings are suppressed.
#
# (The Automoc warning mistakenly hints at removing a necessary #include)
#
list(APPEND CMAKE_AUTOMOC_MACRO_NAMES "DEFINE_VALIDATOR_TEST")

ecm_add_tests(
    ${validator_lib_test_SRCS}
    LINK_LIBRARIES
        Qt5::Core Qt5::Gui Qt5::Test
        validator_lib
    NAME_PREFIX validators-
)
