# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(LIB_SRC
	diff.h
	diff.cpp)

find_package(Qt6Core REQUIRED)

add_library(md-utils STATIC ${LIB_SRC})

target_link_libraries(md-utils Qt6::Core)
