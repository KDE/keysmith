#!/bin/sh
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set -e

cat << INTRO
Running android-export.sh wrapper script: $0
Build info

== target:

 - api  : @CMAKE_ANDROID_API@
 - arch : @CMAKE_ANDROID_ARCH@
 - abi  : @CMAKE_ANDROID_ARCH_ABI@
 - stl  : @CMAKE_ANDROID_STL_TYPE@

== build host:

 - host: @CMAKE_HOST_SYSTEM_NAME@
 - arch: @CMAKE_HOST_SYSTEM_PROCESSOR@

== ndk:

 - path : @CMAKE_ANDROID_NDK@

INTRO

tool_prefix ()
{
    local binutils="$1"
    # see: https://developer.android.com/ndk/guides/other_build_systems
    case "@CMAKE_ANDROID_ARCH@" in
        arm)
            if [ -n "$binutils" ]
            then
                echo -n "arm-linux-androideabi"
            else
                echo -n "armv7a-linux-androideabi"
            fi
        ;;
        arm64)
            echo -n "aarch64-linux-android"
        ;;
        x86)
            echo -n "i686-linux-android"
        ;;
        x86-64|x86_64) # not sure which will be passed, accept both
            echo -n "x86_64-linux-android"
        ;;
        *)
            echo "Unable to continue: unknown/undefined/unsupported Android architecture: '@CMAKE_ANDROID_ARCH@'" >&2
            exit 254
        ;;
    esac
}

clang_toolname_prefix ()
{
    echo -n "$(tool_prefix)@CMAKE_ANDROID_API@"
}

toolchain_host ()
{
    echo -n "@CMAKE_HOST_SYSTEM_NAME@-@CMAKE_HOST_SYSTEM_PROCESSOR@" | tr [:upper:] [:lower:]
}

toolchain_path ()
{
    echo -n "@CMAKE_ANDROID_NDK@/toolchains/llvm/prebuilt/$(toolchain_host)"
}

get_stl ()
{
    case "@CMAKE_ANDROID_STL_TYPE@" in
        c++_shared)
            echo -n "libc++"
        ;;
        *)
            echo "Unable to continue: unknown/undefined/unsupported STL: '@CMAKE_ANDROID_STL_TYPE@'" >&2
            exit 254
        ;;
    esac
}

get_cxx_flags ()
{
    case "$CXXFLAGS" in
        *"--stl"*)
            echo -n "$CXXFLAGS"
        ;;
        *)
            echo -n "$CXXFLAGS --stl=$(get_stl)"
        ;;
    esac
}


# see: https://developer.android.com/ndk/guides/other_build_systems
BINUTILS_PREFIX="$(tool_prefix "true")"
CLANG_PREFIX="$(clang_toolname_prefix)"
TOOLCHAIN_BINDIR="$(toolchain_path)/bin"
CXXFLAGS="$(get_cxx_flags)"

AR="$TOOLCHAIN_BINDIR/$BINUTILS_PREFIX-ar"
AS="$TOOLCHAIN_BINDIR/$BINUTILS_PREFIX-as"
CC="$TOOLCHAIN_BINDIR/$CLANG_PREFIX-clang"
LD="$TOOLCHAIN_BINDIR/$BINUTILS_PREFIX-ld"
CXX="$TOOLCHAIN_BINDIR/$CLANG_PREFIX-clang++"
STRIP="$TOOLCHAIN_BINDIR/$BINUTILS_PREFIX-strip"
RANLIB="$TOOLCHAIN_BINDIR/$BINUTILS_PREFIX-ranlib"

if [ ! -d "$TOOLCHAIN_BINDIR" ]
then
    echo "Unable to continue: tools directory not found: $TOOLCHAIN_BINDIR" >&2
    echo "Android NDK root directory is supposed to be: @CMAKE_ANDROID_NDK@" >&2
    exit 1
fi

set -x

export PATH="$TOOLCHAIN_BINDIR:$PATH" \
    CXXFLAGS="$CXXFLAGS" \
    AR="$AR" \
    AS="$AS" \
    CC="$CC" \
    LD="$LD" \
    CXX="$CXX" \
    STRIP="$STRIP" \
    RANLIB="$RANLIB"

"$@"
