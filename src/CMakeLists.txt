#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
# SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
# SPDX-FileCopyrightText: 2019-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
# SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
#

add_subdirectory(base32)
add_subdirectory(uri)
add_subdirectory(hmac)
add_subdirectory(oath)
add_subdirectory(secrets)
add_subdirectory(backups)
add_subdirectory(account)
add_subdirectory(model)
add_subdirectory(validators)
add_subdirectory(app)
add_subdirectory(scanner)

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h")

#
# The features header should be namespaced, or at least it *must not* be called 'features.h'.
# Reason is that the GNU C++ standard library implementation also refers to a 'features.h'
# and compilation will blow up very badly inside 'os_defines.h' if it does not get the
# 'features.h' it was (really) looking for.
#
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/keysmith-features.h.in" "${CMAKE_CURRENT_BINARY_DIR}/keysmith-features.h")

add_executable(keysmith main.cpp
	contents/ui/QrCodeImageScanner.qml)

ecm_add_qml_module(keysmith URI "org.kde.keysmith" VERSION 1.0)

ecm_target_qml_sources(keysmith SOURCES
    contents/ui/Main.qml
    contents/ui/AccountsOverview.qml
    contents/ui/AddAccount.qml
    contents/ui/ImportAccount.qml
    contents/ui/ErrorPage.qml
    contents/ui/RenameAccount.qml
    contents/ui/AccountNameForm.qml
    contents/ui/HOTPDetailsForm.qml
    contents/ui/TOTPDetailsForm.qml
    contents/ui/UnlockAccounts.qml
    contents/ui/SetupPassword.qml
    contents/ui/TokenEntryViewLabels.qml
    contents/ui/AccountEntryViewBase.qml
    contents/ui/HOTPAccountEntryView.qml
    contents/ui/TOTPAccountEntryView.qml
    contents/ui/KeysmithGlobalDrawer.qml
	contents/ui/QrCodeScanner.qml
	contents/ui/QrCodeImageScanner.qml
)

target_link_libraries(keysmith
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Svg
        Qt6::QuickControls2
        KF6::CoreAddons
        KF6::I18n
        ${Qca_LIBRARY}
        keysmith_lib
		scanner_lib
)

if (ENABLE_DBUS_INTERFACE)
    target_link_libraries(keysmith PUBLIC KF6::DBusAddons)
endif()

if(ANDROID)
    kirigami_package_breeze_icons(ICONS
        application-quit
        answer-correct
        edit-delete
        edit-undo
        go-next
        list-add
        unlock
        lock
    )

    ecm_add_android_apk(keysmith ANDROID_DIR ${CMAKE_SOURCE_DIR}/android)
else()
    target_link_libraries(keysmith PUBLIC Qt6::Widgets)
endif()

install(TARGETS keysmith ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
