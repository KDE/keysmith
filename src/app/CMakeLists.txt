#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(keysmith_SRCS
    keysmith.cpp
    cli.cpp
    flows_p.cpp
    state_p.cpp
    vms.cpp
)

add_library(keysmith_lib STATIC ${keysmith_SRCS})
target_link_libraries(keysmith_lib
    Qt5::Core Qt5::Gui Qt5::Qml Qt5::Concurrent
    KF5::I18n
    model_lib account_lib
)

if (ENABLE_DBUS_INTERFACE)
    target_link_libraries(keysmith_lib KF5::DBusAddons KF5::WindowSystem)
endif()
