#!/bin/sh
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
#

set -ex

#
# Autotools builds are in-source which is messy. In particular, it can
# cause conflicts if builds have to be run with different ./configure
# settings one after another
#
# Impose some sanity by ensuring builds/configure always start from scratch
#
git clean -fdx

./autogen.sh
./configure "$@"
