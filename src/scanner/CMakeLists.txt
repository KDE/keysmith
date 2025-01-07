#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Badin Nicolas <n1coc4cola@gmail.com>
#

set(scanner_SRCS
    qrcode.cpp
)

add_library(scanner_lib STATIC ${scanner_SRCS})
target_link_libraries(scanner_lib Qt::Core Qt::Qml Qt::Multimedia KF6::PrisonScanner account_lib validator_lib uri_lib)
