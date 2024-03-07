if(GROUP_MODE STREQUAL "GROUP_USE_GROUP")
    set(_deprecation_macros_base_name "LIBGROUP")
else()
    set(_deprecation_macros_base_name "LIBRARY")
endif()

function(testAPI code_var_name)
    set(options BUILD_TIME_ONLY_DISABLABLE NO_WARNING)
    set(oneValueArgs DEPRECATED_AT CXX_STANDARD)
    set(multiValueArgs)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if((NOT ARGS_DEPRECATED_AT) OR
       (ARGS_BUILD_TIME_ONLY_DISABLABLE AND ARGS_DEPRECATED_AT VERSION_GREATER DEPRECATED_EXCLUDED_BEFORE_AND_AT))
       set(_build_result_expected TRUE)
    else()
       set(_build_result_expected FALSE)
    endif()

    if (ARGS_CXX_STANDARD)
        if(MSVC)
            # C++11 is default for MSVC, no /std:c++11 flag available to set
            if (NOT ARGS_CXX_STANDARD STREQUAL "11")
                set(std_flag  "/std:c++${ARGS_CXX_STANDARD}")
            endif()
        else()
            set(std_flag "-std=c++${ARGS_CXX_STANDARD}")
        endif()
    else()
        set(std_flag)
    endif()

    set(CMAKE_REQUIRED_FLAGS "${std_flag}")
    set(CMAKE_REQUIRED_LIBRARIES library)
    set(CMAKE_REQUIRED_DEFINITIONS "-D${_deprecation_macros_base_name}_NO_DEPRECATED")

    set(_code "
#include <library.hpp>
int main(int, char**)
{
    ${${code_var_name}}
}
")

    unset(_result CACHE) # clear out as check_cxx_source_compiles caches the result
    check_cxx_source_compiles("${_code}" _result)

    assert_var_bool_value(_result ${_build_result_expected})

    # check warning
    if(_build_result_expected)
        if(ARGS_BUILD_TIME_ONLY_DISABLABLE AND NOT ARGS_NO_WARNING)
            set(_dep_warning_as_error_result_expected FALSE)
        else()
            set(_dep_warning_as_error_result_expected TRUE)
        endif()

        if(MSVC)
            # warning C4996 warns about deprecated declarations
            set(dep_warning_as_error_flag "-we4996")
        else()
            set(dep_warning_as_error_flag "-Werror=deprecated-declarations")
        endif()

        set(CMAKE_REQUIRED_FLAGS "${std_flag} ${dep_warning_as_error_flag}")
        set(CMAKE_REQUIRED_DEFINITIONS) # unset LIBRARY_DISABLE_DEPRECATED_BEFORE_AND_AT, as LIBRARY_DEPRECATED_WARNINGS_SINCE defaults to it
        unset(_dep_warning_result CACHE) # clear out as check_cxx_source_compiles caches the result
        check_cxx_source_compiles("${_code}" _dep_warning_result)
        assert_var_bool_value(_dep_warning_result ${_dep_warning_as_error_result_expected})
    endif()
endfunction()
