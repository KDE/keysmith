#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set(secret_SRCS secrets.cpp)

add_library(secrets_lib STATIC ${secret_SRCS})

if (BUILD_EXTERNAL)
    message(STATUS "Will build external project: libsodium")
    include(ExternalProject)

    #
    # FIXME: for now assume that ANDROID means cross-compiling.
    # Technically this is not quite true when running on Android itself.
    #
    # As a work-around, a user may set an empty AUTOTOOLS_HOST to disable this.
    #
    if (ANDROID AND NOT DEFINED AUTOTOOLS_HOST)
        # see: https://developer.android.com/ndk/guides/other_build_systems
        set(AUTOTOOLS_HOST "")
        if ("${CMAKE_ANDROID_ARCH}" STREQUAL "arm")
            # note: this is the binutils name, armv7a-linux-androideabi is the name for clang
            set(AUTOTOOLS_HOST "arm-linux-androideabi")
        elseif ("${CMAKE_ANDROID_ARCH}" STREQUAL "arm64")
            set(AUTOTOOLS_HOST "aarch64-linux-android")
        elseif("${CMAKE_ANDROID_ARCH}" STREQUAL "x86")
            set(AUTOTOOLS_HOST "i686-linux-android")
        # not sure which will be passed, accept both
        elseif("${CMAKE_ANDROID_ARCH}" STREQUAL "x86_64" OR "${CMAKE_ANDROID_ARCH}" STREQUAL "x86-64")
            set(AUTOTOOLS_HOST "x86_64-linux-android")
        else()
            message(FATAL_ERROR "Building for Android but got an unknown/undefined/unsupported Android architecture (ANDROID_ARCH): '${ANDROID_ARCH}'")
        endif()
    endif()

    if (AUTOTOOLS_HOST)
        set(AUTOTOOLS_HOST_OPTION "--host=${AUTOTOOLS_HOST}")
    else()
        set(AUTOTOOLS_HOST_OPTION "")
    endif()

    set(sodium_LIBRARY_PATH "lib/libsodium.so")
    set(sodium_INCLUDE_PATH "include")

    #
    # Use a wrapper script for Android to pre-populate the environment with necessary environment variables.
    # This ensures the right toolchain will be picked up by the autotools build system.
    #
    set(ENV_WRAPPER "")
    if (ANDROID)
        set(ENV_WRAPPER "${CMAKE_BINARY_DIR}/android/android-export.sh")
    endif()

    #
    # Make gets confused if there is another make process somewhere in the process tree above it. This prevents it from
    # doing parallel make with the "jobserver is unavailable" warning instead of assuming it is an actual top-level make
    # process as it were.
    #
    # The work-around is to use $(MAKE), but that doesn't work if the parent process is not itself a make (e.g. ninja).
    # So the work-around for that is to use a wrapper script by default that does the right thing, except when CMake is
    # configured to use the Unix Makefiles generator in which case $(MAKE) should be used instead.
    #
    set(MAKE_SH "${CMAKE_SOURCE_DIR}/cmake/external/make.sh")
    if ("${CMAKE_GENERATOR}" STREQUAL "Unix Makefiles")
        set(MAKE_SH "$(MAKE)")
    endif()
    set(CONFIGURE_SH "${CMAKE_SOURCE_DIR}/cmake/external/configure-autotools.sh" "${AUTOTOOLS_HOST_OPTION}" "--prefix=<INSTALL_DIR>")

    #
    # Unfortunately CMake generates a build system (make/ninja) which does not check whether the library already exists.
    # This causes the external project to be fully rebuilt always. In turn, that triggers a rebuild of everything from
    # the external project upwards as well because the library file has changed.
    #
    # CHECK_SH is a prefix/wrapper for actual commands that short-circuits the build if the library already exists.
    #
    set(CHECK_SH test -e "<INSTALL_DIR>/${sodium_LIBRARY_PATH}" ||)

    externalProject_add(
        ext_libsodium
        PREFIX "${CMAKE_BINARY_DIR}/external"
        GIT_REPOSITORY https://github.com/jedisct1/libsodium.git
        GIT_TAG 1.0.18 # ${sodium_VERSION_STRING}
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        CONFIGURE_COMMAND ${CHECK_SH} ${ENV_WRAPPER} ${CONFIGURE_SH}
        BUILD_COMMAND ${CHECK_SH} ${ENV_WRAPPER} ${MAKE_SH}
        BUILD_IN_SOURCE ON
        BUILD_ALWAYS OFF
        INSTALL_COMMAND ${CHECK_SH} ${ENV_WRAPPER} ${MAKE_SH} install
        # register the lib manually, otherwise cmake gets very confused about depending on this file
        BUILD_BYPRODUCTS <INSTALL_DIR>/${sodium_LIBRARY_PATH}
    )

    add_library(sodium SHARED IMPORTED)

    externalProject_get_property(ext_libsodium INSTALL_DIR)

    #
    # INTERFACE_INCLUDE_DIRECTORIES cannot take a non-existent directory. However, nearly everything external projects do
    # is done *after* cmake configure stage is over, so the directory simply won't be there.
    #
    # Apply the popular work-around and create it up front.
    #
    file(MAKE_DIRECTORY "${INSTALL_DIR}/${sodium_INCLUDE_PATH}")
    set_target_properties(sodium PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/${sodium_LIBRARY_PATH}" INTERFACE_INCLUDE_DIRECTORIES "${INSTALL_DIR}/${sodium_INCLUDE_PATH}")

    add_dependencies(secrets_lib ext_libsodium)

    #
    # Add an explicit install rule for libsodium: without it a make/ninja install will not pick it up.
    #
    # Note that the androiddeployqt tooling is documented to explicitly require a make/ninja install step:
    # https://doc.qt.io/qt-5/deployment-android.html#package-template
    #
    install(FILES "${INSTALL_DIR}/${sodium_LIBRARY_PATH}" TYPE LIB)
endif()

target_link_libraries(secrets_lib Qt::Core sodium base32_lib)
