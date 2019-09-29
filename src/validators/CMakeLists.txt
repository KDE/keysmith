#
# This directory contains (custom) validators to support form field validation in the QML UI
#

set(validator_SRCS secretvalidator.cpp)

add_library(validator_lib STATIC ${validator_SRCS})
target_link_libraries(validator_lib Qt5::Core Qt5::Gui)
