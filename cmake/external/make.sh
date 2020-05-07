#!/bin/sh
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set -e

#
# Try to speed up builds by using parallel make. Assume that if MAKEFLAGS
# is set, someone is trying to control make behaviour explicitly and do not
# second-guess that.
#
if [ -z "$MAKEFLAGS" ]
then
    cores="$(nproc --all)"
    jobs="$(($cores * 5 / 4))"
    set -x
    make -j "$jobs" "$@"
else
    set -x
    make "$@"
fi
