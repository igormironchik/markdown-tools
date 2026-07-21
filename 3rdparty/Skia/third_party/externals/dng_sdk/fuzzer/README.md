# Fuzzing DNG SDK

This fuzzer is intented to do a varian analysis of the issue reported
in b/156261521.

Here is a list of some CVEs previously discovered in DNG SDK:

* CVE-2020-9589
* CVE-2020-9590
* CVE-2020-9620
* CVE-2020-9621
* CVE-2020-9622
* CVE-2020-9623
* CVE-2020-9624
* CVE-2020-9625
* CVE-2020-9626
* CVE-2020-9627
* CVE-2020-9628
* CVE-2020-9629

## Building & running the fuzz target: Android device

It is recommended to set rss limit to higher values (such as 4096) when running
the fuzzer to avoid frequent OOM libFuzzer crashes.

```sh
$ source build/envsetup.sh
$ lunch aosp_arm64-eng
$ SANITIZE_TARGET=hwaddress make dng_parser_fuzzer
$ adb sync data
$ adb shell /data/fuzz/arm64/dng_parser_fuzzer/dng_parser_fuzzer \
$ -rss_limit=4096 \
$ /data/fuzz/arm64/dng_parser_fuzzer/corpus
```

## Building & running the fuzz target: Host

```sh
$ source build/envsetup.sh
$ lunch aosp_x86_64-eng
$ SANITIZE_HOST=address make dng_parser_fuzzer
$ LD_LIBRARY_PATH=$ANDROID_HOST_OUT/fuzz/x86_64/lib/ \
$ $ANDROID_HOST_OUT/fuzz/x86_64/dng_parser_fuzzer/dng_parser_fuzzer \
$ -rss_limit_mb=4096 \
$ $ANDROID_HOST_OUT/fuzz/x86_64/dng_parser_fuzzer/corpus/
```

