#.rst:
# FindLibOath
# ---------
#
# Try to locate the liboath library.
# If found, this will define the following variables:
#
# ``LIBOATH_FOUND``
#     True if the LibOath library is available
# ``LIBOATH_INCLUDE_DIRS``
#     The LibOath include directories
# ``LIBOATH_LIBRARIES``
#     The LibOath libraries for linking
# ``LIBOATH_INCLUDE_DIR``
#     Deprecated, use ``LIBOATH_INCLUDE_DIRS``
# ``LIBOATH_LIBRARY``
#     Deprecated, use ``LIBOATH_LIBRARIES``
#
# If ``LIBOATH_FOUND`` is TRUE, it will also define the following
# imported target:
#
# ``LIBOATH::LIBOATH``
#     The LIBOATH library
#
#=============================================================================
# Copyright (c) 2019 Bhushan Shah, <bshah@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

find_package(PkgConfig)
pkg_check_modules(PC_LIBOATH QUIET liboath)

find_path(LIBOATH_INCLUDE_DIRS
          NAMES oath.h
          HINTS ${PC_LIBOATH_INCLUDEDIR}
          PATH_SUFFIXES liboath)

find_library(LIBOATH_LIBRARIES
             NAMES oath
             HINTS ${PC_LIBOATH_LIBDIR})

set(LIBOATH_INCLUDE_DIR "${LIBOATH_INCLUDE_DIRS}")
set(LIBOATH_LIBRARY "${LIBOATH_LIBRARIES}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBOATH DEFAULT_MSG LIBOATH_LIBRARIES LIBOATH_INCLUDE_DIRS)

if(LIBOATH_FOUND AND NOT TARGET LIBOATH::LIBOATH)
    add_library(LIBOATH::LIBOATH UNKNOWN IMPORTED)
    set_target_properties(LIBOATH::LIBOATH PROPERTIES
                          IMPORTED_LOCATION "${LIBOATH_LIBRARIES}"
                          INTERFACE_INCLUDE_DIRECTORIES "${LIBOATH_INCLUDE_DIR}")
endif()

mark_as_advanced(LIBOATH_INCLUDE_DIRS LIBOATH_INCLUDE_DIR
                 LIBOATH_LIBRARIES LIBOATH_LIBRARY)

include(FeatureSummary)
set_package_properties(LIBOATH PROPERTIES
  URL "http://www.nongnu.org/oath-toolkit/"
  DESCRIPTION "Library for Open AuTHentication (OATH) HOTP etc support.")
