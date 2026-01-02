# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: MIT

if(NOT RESVG_INCLUDE_DIR)
    find_path(RESVG_INCLUDE_DIR NAMES resvg.h ResvgQt.h)
endif()

find_library(RESVG_LIBRARY NAMES resvg)

if(RESVG_INCLUDE_DIR AND RESVG_LIBRARY)
    set(RESVG_FOUND TRUE)
    set(resvg_FOUND TRUE)
	
    if(NOT TARGET resvg)
        add_library(resvg UNKNOWN IMPORTED)

        target_include_directories(resvg INTERFACE ${RESVG_INCLUDE_DIR})
        set_target_properties(resvg PROPERTIES
            IMPORTED_LOCATION ${RESVG_LIBRARY})
    endif()
endif ()
