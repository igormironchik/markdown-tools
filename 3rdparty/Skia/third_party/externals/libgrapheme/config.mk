# Customize below to fit your system (run ./configure for automatic presets)

# paths (unset $PCPREFIX to not install a pkg-config-file)
DESTDIR   =
PREFIX    = /usr/local
INCPREFIX = $(PREFIX)/include
LIBPREFIX = $(PREFIX)/lib
MANPREFIX = $(PREFIX)/share/man
PCPREFIX  = $(LIBPREFIX)/pkgconfig

# names
ANAME     = libgrapheme.a
SONAME    = libgrapheme.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
BINSUFFIX = 

# flags
CPPFLAGS = -D_ISOC99_SOURCE
CFLAGS   = -std=c99 -Os -Wall -Wextra -Wpedantic
LDFLAGS  = -s

BUILD_CPPFLAGS = $(CPPFLAGS)
BUILD_CFLAGS   = $(CFLAGS)
BUILD_LDFLAGS  = $(LDFLAGS)

SHFLAGS   = -fPIC -ffreestanding
SOFLAGS   = -shared -nostdlib -Wl,--soname=libgrapheme.so.$(VERSION_MAJOR).$(VERSION_MINOR)
SOSYMLINK = true

# tools (unset $LDCONFIG to not call ldconfig(1) after install/uninstall)
CC       = cc
BUILD_CC = $(CC)
AR       = ar
RANLIB   = ranlib
LDCONFIG = ldconfig
SH       = sh
