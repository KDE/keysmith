project(otpclient)


#
# Need 3.10.3 for C++17 support enabled via CMake: https://cmake.org/cmake/help/v3.10/prop_tgt/CXX_STANDARD.html#prop_tgt:CXX_STANDARD
#
cmake_minimum_required(VERSION 3.10.3)
set(KF5_MIN_VERSION "5.18.0")
set(QT_MIN_VERSION "5.5.0")

################# Disallow in-source build #################

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "This application requires an out of source build. Please create a separate build directory.")
endif()

include(FeatureSummary)

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_SOURCE_DIR}/cmake/")

include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMPoQmTools)
include(KDECompilerSettings NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################# Find dependencies #################

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Svg QuickControls2)
find_package(LibOath REQUIRED)
find_package(KF5Kirigami2 ${KF5_MIN_VERSION})

################ Find testing dependencies ##########

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

################# build and install #################
add_subdirectory(src)
add_subdirectory(autotests)

install(PROGRAMS org.kde.otpclient.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.otpclient.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
