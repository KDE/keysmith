#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(validator_lib_test_SRCS
    base32-validator.cpp
    datetime-parsing.cpp
    epoch-validator.cpp
    name-validator.cpp
    issuer-validator.cpp
    unsigned-long-validator.cpp
    unsigned-long-parsing.cpp
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
        Qt::Core Qt::Gui Qt::Test
        validator_lib
    NAME_PREFIX validators-
)
