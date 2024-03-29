cmake_minimum_required(VERSION 3.5)
project(extra-cmake-modules)

# order is important (install dirs before cmake settings)!
set(all_kde_modules
    KDEInstallDirs
    KDECMakeSettings
    ECMMarkNonGuiExecutable
)

set(ECM_KDE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../kde-modules)
set(ECM_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../modules)
set(CMAKE_MODULE_PATH "${ECM_KDE_MODULE_DIR}" "${ECM_MODULE_DIR}")

foreach(module ${all_kde_modules})
    message(STATUS "module: ${module}")
    include("${module}")
endforeach()

_repository_name(name "${CMAKE_CURRENT_SOURCE_DIR}/../../")
if (NOT ${name} MATCHES ".*/extra-cmake-modules")
    message(FATAL_ERROR "Wrong repository name: ${name}, should match '*/extra-cmake-modules'")
endif()
add_executable(dummy ../ExecuteKDEModules/main.c)
ecm_mark_nongui_executable(dummy)

set_target_properties(dummy PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if(NOT TARGET fetch-translations)
    message(FATAL_ERROR "should have a fetch-translations target")
endif()
