#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2019-2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
# SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
#
include_directories(BEFORE ../src)

add_subdirectory(test-utils)
add_subdirectory(base32)
add_subdirectory(uri)
add_subdirectory(hmac)
add_subdirectory(oath)
add_subdirectory(secrets)
add_subdirectory(account)
add_subdirectory(model)
add_subdirectory(validators)
add_subdirectory(app)
