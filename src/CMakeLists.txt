#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
# SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
# SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
#

add_subdirectory(base32)
add_subdirectory(uri)
add_subdirectory(hmac)
add_subdirectory(oath)
add_subdirectory(secrets)
add_subdirectory(account)
add_subdirectory(model)
add_subdirectory(validators)
add_subdirectory(app)

set(keysmith_SRCS
    main.cpp
)

set(keysmith_internal_libs keysmith_lib)

qt5_add_resources(RESOURCES resources.qrc)
add_executable(keysmith ${keysmith_SRCS} ${RESOURCES})
target_link_libraries(keysmith
    Qt5::Core Qt5::Qml Qt5::Quick Qt5::Svg
    KF5::I18n
    ${keysmith_internal_libs}
)

if(ANDROID)
    kirigami_package_breeze_icons(ICONS
        answer-correct
        edit-delete
        edit-undo
        go-next
        list-add
        unlock
    )
endif()

install(TARGETS keysmith ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
