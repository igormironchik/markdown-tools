include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::Test)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

ecm_add_tests(
    rfc2047test.cpp
    base45test.cpp
    codectest.cpp
    kemailaddresstest.cpp
    LINK_LIBRARIES KF6::Codecs Qt6::Test
)

ecm_add_test(
    kcharsetstest.cpp
    LINK_LIBRARIES KF6::Codecs Qt6::Test
)

ecm_add_test(
    TEST_NAME kencodingproberunittest
    kencodingproberunittest.cpp
    ../src/probers/nsCharSetProber.cpp
    ../src/probers/nsLatin1Prober.cpp
    ../src/probers/nsHebrewProber.cpp
    ../src/probers/nsSBCharSetProber.cpp
    ../src/probers/LangHebrewModel.cpp
    LINK_LIBRARIES Qt6::Test
)
target_include_directories(kencodingproberunittest PRIVATE ${CMAKE_BINARY_DIR}/src)

add_executable(kencodingprobertest_bin
    kencodingprobertest.cpp
)
target_link_libraries(kencodingprobertest_bin
    Qt6::Test
    KF6::Codecs
)
ecm_mark_as_test(kencodingprobertest_bin)
ecm_mark_nongui_executable(kencodingprobertest_bin)
# Run the benchmarks with just 1 iteration, so we known it works
add_test(NAME kencodingprobertest COMMAND kencodingprobertest_bin "-iterations" "1")

# Benchmark, compiled, but not run automatically with ctest
add_executable(base64benchmark base64benchmark.cpp)
target_link_libraries(base64benchmark KF6::Codecs Qt6::Test)
