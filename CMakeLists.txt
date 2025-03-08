# SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
# SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
# SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "25")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(keysmith VERSION ${RELEASE_SERVICE_VERSION})

set(KF_MIN_VERSION "6.8.0")
set(QT_MIN_VERSION "6.5.0")
set(SODIUM_MIN_VERSION "1.0.16")

include(FeatureSummary)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddTests)
include(ECMPoQmTools)
include(ECMAddAndroidApk)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMQmlModule)
include(ECMDeprecationSettings)
include(ECMFindQmlModule)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Gui Svg QuickControls2 Concurrent)
find_package(KF6Kirigami ${KF_MIN_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_MIN_VERSION} REQUIRED)
find_package(KF6I18n ${KF_MIN_VERSION} REQUIRED)
find_package(KF6Config ${KF_MIN_VERSION} REQUIRED)

ecm_find_qmlmodule(org.kde.prison.scanner REQUIRED)

ecm_set_disabled_deprecation_versions(QT 6.0.0
    KF 6.12.0
)


if(NOT ANDROID)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)
endif()

if(NOT BUILD_EXTERNAL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

    message(STATUS "Will build against system version of: libsodium")
    find_package(sodium ${SODIUM_MIN_VERSION} REQUIRED)
endif()

find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES
    TYPE REQUIRED
    PURPOSE "Encrypted communications"
)

if (BUILD_DBUS_INTERFACE OR (NOT ANDROID AND NOT DEFINED BUILD_DBUS_INTERFACE))
    find_package(KF6DBusAddons ${KF_MIN_VERSION} REQUIRED)
    find_package(KF6WindowSystem ${KF_MIN_VERSION} REQUIRED)
    set(ENABLE_DBUS_INTERFACE ON)
endif()

if (NOT ANDROID)
    find_package(KF6QQC2DesktopStyle ${KF_MIN_VERSION} REQUIRED)
    set_package_properties(KF6QQC2DesktopStyle PROPERTIES
        TYPE RUNTIME
    )
endif()

add_subdirectory(src)

if (ANDROID)
    add_subdirectory(android)
endif()

if (BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Test)
    add_subdirectory(autotests)
endif()

install(FILES org.kde.keysmith.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.keysmith.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps/)
install(FILES org.kde.keysmith.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

ki18n_install(po)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
