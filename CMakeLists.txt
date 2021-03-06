#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019 Bhushan Shah <bshah@kde.org>
# SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
# SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
#

# Need 3.10.X for C++17 support enabled via CMake: https://cmake.org/cmake/help/v3.10/prop_tgt/CXX_STANDARD.html#prop_tgt:CXX_STANDARD
cmake_minimum_required(VERSION 3.10)

project(keysmith)
set(PROJECT_VERSION "21.06")

set(KF5_MIN_VERSION "5.77.0")
set(QT_MIN_VERSION "5.15.0")
set(SODIUM_MIN_VERSION "1.0.16")

################# Disallow in-source build #################

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "This application requires an out of source build. Please create a separate build directory.")
endif()

include(FeatureSummary)

################# set KDE specific information #################

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddTests)
include(ECMPoQmTools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################# Find dependencies #################

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Gui Svg QuickControls2 Concurrent)
find_package(KF5Kirigami2 ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_MIN_VERSION} REQUIRED)

if(NOT BUILD_EXTERNAL AND (NOT ANDROID OR DEFINED BUILD_EXTERNAL))
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

    message(STATUS "Will build against system version of: libsodium")
    find_package(sodium ${SODIUM_MIN_VERSION} REQUIRED)
endif()

if (BUILD_DBUS_INTERFACE OR (NOT ANDROID AND NOT DEFINED BUILD_DBUS_INTERFACE))
    find_package(KF5DBusAddons ${KF5_MIN_VERSION} REQUIRED)
    find_package(KF5WindowSystem ${KF5_MIN_VERSION} REQUIRED)
    set(ENABLE_DBUS_INTERFACE ON)
endif()

################ Find testing dependencies ##########

if (BUILD_TESTING)
    find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Test)
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
