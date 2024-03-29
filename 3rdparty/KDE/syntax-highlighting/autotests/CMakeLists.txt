#
# test configuration
#
configure_file(test-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/test-config.h)

#
# helper to update the references for the unit tests
#
configure_file(update-reference-data.sh.in ${CMAKE_CURRENT_BINARY_DIR}/update-reference-data.sh)

#
# unit tests
# always executed during testing, required to pass, else stuff is in a bad shape ;=)
#
add_executable(wildcardmatcher_test wildcardmatcher_test.cpp)
ecm_mark_nongui_executable(wildcardmatcher_test)
add_test(NAME wildcardmatcher_test COMMAND wildcardmatcher_test)
target_link_libraries(wildcardmatcher_test KF6SyntaxHighlighting Qt6::Test)

add_executable(repository_test repository_test_base.cpp repository_test.cpp)
ecm_mark_nongui_executable(repository_test)
add_test(NAME repository_test COMMAND repository_test)
target_link_libraries(repository_test KF6SyntaxHighlighting Qt6::Test)

add_executable(theme_test theme_test.cpp)
ecm_mark_nongui_executable(theme_test)
add_test(NAME theme_test COMMAND theme_test)
target_link_libraries(theme_test KF6SyntaxHighlighting Qt6::Test)

add_executable(testhighlighter_test testhighlighter.cpp)
ecm_mark_nongui_executable(testhighlighter_test)
add_test(NAME testhighlighter_test COMMAND testhighlighter_test)
target_link_libraries(testhighlighter_test KF6SyntaxHighlighting Qt6::Test)

add_executable(htmlhighlighter_test htmlhighlighter_test.cpp)
ecm_mark_nongui_executable(htmlhighlighter_test)
add_test(NAME htmlhighlighter_test COMMAND htmlhighlighter_test)
target_link_libraries(htmlhighlighter_test KF6SyntaxHighlighting Qt6::Test)

add_executable(folding_test foldingtest.cpp)
ecm_mark_nongui_executable(folding_test)
add_test(NAME folding_test COMMAND folding_test)
target_link_libraries(folding_test KF6SyntaxHighlighting Qt6::Test)

#
# benchmarks, don't execute during normal testing
# they don't test additional stuff but only hang the CI
#
add_executable(highlighter_benchmark highlighter_benchmark.cpp)
ecm_mark_nongui_executable(highlighter_benchmark)
add_test(NAME highlighter_benchmark COMMAND highlighter_benchmark CONFIGURATIONS BENCHMARK)
target_link_libraries(highlighter_benchmark KF6SyntaxHighlighting Qt6::Test)

add_executable(repository_benchmark repository_test_base.cpp repository_benchmark.cpp)
ecm_mark_nongui_executable(repository_benchmark)
add_test(NAME repository_benchmark COMMAND repository_benchmark CONFIGURATIONS BENCHMARK)
target_link_libraries(repository_benchmark KF6SyntaxHighlighting Qt6::Test)
