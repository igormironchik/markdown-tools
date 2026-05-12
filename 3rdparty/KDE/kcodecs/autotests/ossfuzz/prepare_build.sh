#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2019 Google Inc.
# SPDX-License-Identifier: Apache-2.0
#
# Based on https://github.com/google/oss-fuzz/blob/33aab4a70dc4b5811143d214536584a8c8cb3924/projects/kcodecs/Dockerfile
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

apt-get update && apt-get install --yes cmake ninja-build

git clone --depth 1 --branch=dev git://code.qt.io/qt/qtbase.git
git clone --depth 1 -b master https://invent.kde.org/frameworks/extra-cmake-modules.git
git clone --depth 1 https://gitlab.freedesktop.org/uchardet/uchardet.git
