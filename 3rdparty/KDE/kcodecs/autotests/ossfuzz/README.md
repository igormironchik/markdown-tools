<!-- SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net> -->
<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->

# KCodecs OSS-Fuzz Integration

## Fuzzing Locally
Make sure you're using Clang (by setting `CC` and `CXX`), since fuzzing requires it. Then build KCodecs with `BUILD_FUZZERS=ON` to generate the `kcodecs_fuzzer` binary:
```sh
cmake -B build -DBUILD_FUZZERS=ON
cmake --build build
./build/bin/fuzzers/kcodecs_fuzzer
```

## Testing OSS-Fuzz Integration
Testing OSS-Fuzz integration requires: Python & Docker

First clone the OSS-Fuzz repository:
```sh
git clone https://github.com/google/oss-fuzz.git
```

After navigating to the cloned repository, run the following command to build the fuzzers:
```sh
python3 infra/helper.py build_image kcodecs
python3 infra/helper.py build_fuzzers --sanitizer address kcodecs
```

This may take a while since it builds the whole QtBase dependency alongside with KCodecs. Once the build is completed, you can run the fuzzers using the following command:
```sh
python3 infra/helper.py run_fuzzer kcodecs kcodecs_fuzzer
```

The code for preparing the build lives in the `prepare_build.sh` script and the code for building the fuzzers lives in the `build_fuzzers.sh` script (which is also responsible for building the dependencies, creating the seed corpus and copying the dict file).

For more information on OSS-Fuzz, visit the [official website](https://google.github.io/oss-fuzz/).
