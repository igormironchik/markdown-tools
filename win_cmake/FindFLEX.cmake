# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: MIT

if(DEFINED ENV{LEX})
    set(FLEX_EXECUTABLE $ENV{LEX})
    set(FLEX_FOUND TRUE)
endif()
