#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Badin Nicolas <n1coc4cola@gmail.com>
#

set(scanner_SRCS
	qrcodevideo.cpp
	qrcodeimage.cpp
)

add_library(scanner_lib STATIC ${scanner_SRCS})
target_link_libraries(scanner_lib Qt::Core Qt::Gui Qt::Concurrent Qt::Multimedia KF6::PrisonScanner)
