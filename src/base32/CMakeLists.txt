#
# This directory contains a wrapper around base32 functionality of the oath library
#

set(base32_SRCS base32.cpp)

add_library(base32_lib STATIC ${base32_SRCS})
target_link_libraries(base32_lib Qt5::Core ${LIBOATH_LIBRARIES})
