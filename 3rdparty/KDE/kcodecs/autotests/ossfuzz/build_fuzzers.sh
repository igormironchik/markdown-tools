#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2019 Google Inc.
# SPDX-License-Identifier: Apache-2.0
#
# Copyright 2019 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

rm -rf $WORK/*

export PATH="$WORK/bin:$PATH"
if [[ $FUZZING_ENGINE == "afl" ]]; then
    export LDFLAGS="-fuse-ld=lld"
fi

cd $SRC/extra-cmake-modules
cmake . -G Ninja \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=$WORK
ninja install -j$(nproc)

cd $SRC/qtbase
./configure -no-glib -qt-libpng -qt-pcre -qt-zlib -opensource -confirm-license -static -no-opengl \
    -no-icu -platform linux-clang-libc++ -debug -prefix $WORK -no-feature-gui -no-feature-sql \
    -no-feature-network -no-feature-xml -no-feature-dbus -no-feature-printsupport
ninja install -j$(nproc)

cd $SRC/kcodecs
rm -rf poqm
cmake . -G Ninja \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTING=OFF \
    -DBUILD_FUZZERS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=$WORK
ninja install -j$(nproc)

# Copy the fuzzer
cp bin/fuzzers/kcodecs_fuzzer $OUT/kcodecs_fuzzer

# Create a seed corpus
zip -qr $OUT/kcodecs_fuzzer_seed_corpus.zip $SRC/uchardet/test/ $SRC/kcodecs/autotests/data
