#ifndef MY_OPENSSL_CONF_H
#define MY_OPENSSL_CONF_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif // __APPLE__

#if defined(_WIN64)
#include "Win64/opensslconf.h"
#elif defined(_WIN32)
#include "Win32/opensslconf.h"
#elif defined(__linux__)
#include "linux-x86_64/opensslconf.h"
#elif defined(__APPLE__) && defined(TARGET_OS_MAC)
#include "macosx-x86_64/opensslconf.h"
#elif defined(__APPLE__) && defined(TARGET_OS_IPHONE) && defined(TARGET_IPHONE_SIMULATOR)
#include "ios-x86_64/opensslconf.h"
#elif defined(__APPLE__) && defined(TARGET_OS_IPHONE)
#include "ios-arm64/opensslconf.h"
#elif defined(__ANDROID__) && defined(__arm__)
#include "android-arm/opensslconf.h"
#elif defined(__ANDROID__) && defined(__aarch64__)
#include "android-arm64/opensslconf.h"
#elif defined(__ANDROID__) && defined(__x86_64__)
#include "android-x86_64/opensslconf.h"
#else
#error Unsupported architecture
#endif

#endif // MY_OPENSSL_CONF_H