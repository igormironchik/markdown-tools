cmake_minimum_required( VERSION 3.19 )

find_package(Qt6 REQUIRED COMPONENTS Core)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(katehighlightingindexer katehighlightingindexer.cpp ../lib/worddelimiters.cpp)
target_link_libraries(katehighlightingindexer Qt6::Core)
