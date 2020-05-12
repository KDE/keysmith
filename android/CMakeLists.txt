#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/android-export.in.sh" "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/android-export.sh" @ONLY ESCAPE_QUOTES)

file (
    COPY "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/android-export.sh"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

#
# The KDE Android SDK overwrites the AndroidManifest to set an auto-generated versionCode.
# When building against a local clone, this results in annoying git diffs that must be manually reset.
#
# As a work-around, the AndroidManifest.xml is gitignore'd and generated (copied) from another file, taking care to
# preserve a file-level dependency between the 'real' source and the generated file.
#
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/AndroidManifest.xml.in" "${CMAKE_CURRENT_SOURCE_DIR}/AndroidManifest.xml" COPYONLY)
