/*
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
*/

/*
  Usage:
    python infra/helper.py build_image kcodecs
    python infra/helper.py build_fuzzers --sanitizer undefined|address|memory kcodecs
    python infra/helper.py run_fuzzer kcodecs kcodecs_fuzzer
*/

#include <QCoreApplication>
#include <QVector>

#include <kcodecs.h>
#include <kencodingprober.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    int argc = 0;
    QCoreApplication a(argc, nullptr);

    const QByteArray ba((const char *)data, size);
    const std::unique_ptr<KEncodingProber> prober = std::make_unique<KEncodingProber>();

    // Arabic, Baltic, CentralEuropean, Cyrillic, Greek, Hebrew, NorthernSaami, Other,
    // SouthEasternEurope, Thai, Turkish, WesternEuropean
    // All of these use nsSBCSGroupProber, so add only Arabic to avoid duplication
    // ChineseSimplified and ChineseTraditional both use ChineseGroupProber
    const QVector<KEncodingProber::ProberType> allProbers = {
        KEncodingProber::None,
        KEncodingProber::Universal,
        KEncodingProber::Arabic,
        KEncodingProber::ChineseTraditional,
        KEncodingProber::Japanese,
        KEncodingProber::Korean,
        KEncodingProber::Unicode
    };
    for (KEncodingProber::ProberType type : allProbers) {
        prober->setProberType(type);
        prober->feed(ba);
        prober->reset();
    }

    const QVector<const char *> codecs = {"base64", "quoted-printable", "b", "q", "x-kmime-rfc2231", "x-uuencode"};
    for (const char *codecName : codecs) {
        KCodecs::Codec *c = KCodecs::Codec::codecForName(codecName);
        c->encode(ba, KCodecs::Codec::NewlineCRLF);
        c->decode(ba, KCodecs::Codec::NewlineCRLF);
        c->encode(ba, KCodecs::Codec::NewlineLF);
        c->decode(ba, KCodecs::Codec::NewlineLF);
    }

    return 0;
}
