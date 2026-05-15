#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
# SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: LGPL-2.0-or-later

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
./configure -platform linux-clang-libc++ -prefix $WORK -static -opensource -confirm-license -debug \
    -qt-pcre -qt-zlib \
    -no-glib -no-icu -no-feature-gui -no-feature-sql -no-feature-network \
    -no-feature-dbus -no-feature-printsupport -no-feature-widgets
ninja install -j$(nproc)

cd $SRC/kconfig
rm -rf poqm
cmake . -G Ninja \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTING=OFF \
    -DBUILD_FUZZERS=ON \
    -DKCONFIG_USE_GUI=OFF \
    -DKCONFIG_USE_QML=OFF \
    -DUSE_DBUS=OFF \
    -DCMAKE_INSTALL_PREFIX=$WORK
ninja install -j$(nproc)

# Copy the fuzzer
cp bin/fuzzers/kconfig_fuzzer $OUT/kconfig_fuzzer
# Create a seed corpus
find . -name "*.config" | zip -q $OUT/kconfig_fuzzer_seed_corpus.zip -@
# Copy the dictionary
cp autotests/ossfuzz/kconfig_fuzzer.dict $OUT/kconfig_fuzzer.dict
