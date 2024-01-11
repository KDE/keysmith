# SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
# SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
# SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "04")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(keysmith VERSION ${RELEASE_SERVICE_VERSION})

set(KF_MIN_VERSION "5.240.0")
set(QT_MIN_VERSION "6.5.0")
set(SODIUM_MIN_VERSION "1.0.16")
set(KDE_COMPILERSETTINGS_LEVEL "5.240.0")

################# Disallow in-source build #################

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "This application requires an out of source build. Please create a separate build directory.")
endif()

include(FeatureSummary)

################# set KDE specific information #################

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddTests)
include(ECMPoQmTools)
include(ECMAddAndroidApk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################# Find dependencies #################

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Gui Svg QuickControls2 Concurrent)
find_package(KF6Kirigami ${KF_MIN_VERSION} REQUIRED)
find_package(KF6I18n ${KF_MIN_VERSION} REQUIRED)

if(NOT ANDROID)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)
endif()

find_package(Qt6Core5Compat REQUIRED)

if(NOT BUILD_EXTERNAL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

    message(STATUS "Will build against system version of: libsodium")
    find_package(sodium ${SODIUM_MIN_VERSION} REQUIRED)
endif()

if (BUILD_DBUS_INTERFACE OR (NOT ANDROID AND NOT DEFINED BUILD_DBUS_INTERFACE))
    find_package(KF6DBusAddons ${KF_MIN_VERSION} REQUIRED)
    find_package(KF6WindowSystem ${KF_MIN_VERSION} REQUIRED)
    set(ENABLE_DBUS_INTERFACE ON)
endif()

################ Find testing dependencies ##########

if (BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Test)
endif()

################# build and install #################
add_subdirectory(src)

if (ANDROID)
    add_subdirectory(android)
endif()

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(PROGRAMS org.kde.keysmith.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES keysmith.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps/)
install(FILES org.kde.keysmith.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

################# install translations #################
ki18n_install(po)
