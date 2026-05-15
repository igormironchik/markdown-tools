remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Concurrent)

if (BUILD_WITH_QML)
    ecm_add_test(ki18ndeclarativetest.cpp)
    target_link_libraries(ki18ndeclarativetest PRIVATE Qt6::Test Qt6::Qml KF6::I18n KF6::I18nQml)
    qt6_add_qml_module(ki18ndeclarativetest URI "org.kde.i18n.declarativetest" QML_FILES Test.qml)
    if (NOT QT6_IS_SHARED_LIBS_BUILD)
        qt6_import_qml_plugins(ki18ndeclarativetest)
    endif()
endif()

ecm_add_test(klocalizedstringtest.cpp
    TEST_NAME "ki18n-klocalizedstringtest"
    LINK_LIBRARIES Qt6::Test Qt6::Concurrent KF6::I18n ${LibIntl_LIBRARIES}
)
# klocalizedstringtest needs the libintl include path
target_include_directories(ki18n-klocalizedstringtest PRIVATE ${LibIntl_INCLUDE_DIRS})

ecm_add_test(klazylocalizedstringtest.cpp
    TEST_NAME "ki18n-klazylocalizedstringtest"
    LINK_LIBRARIES Qt6::Test KF6::I18n
)

if (TARGET ktranscript)
ecm_add_test(ktranscripttest.cpp testhelpers.cpp
    TEST_NAME "ki18n-ktranscripttest"
    LINK_LIBRARIES Qt6::Test KF6::I18n
)
# ktranscripttest needs the path to ktranscript in the build dir: it cannot rely
# on the installed version since it must be able to run *before* the plugin is
# installed.
target_compile_definitions(ki18n-ktranscripttest PRIVATE "KTRANSCRIPT_PATH=\"$<TARGET_FILE:ktranscript>\"")

# ktranscriptcleantest needs to directly compile ktranscript.cpp with an addition DEFINE
ecm_add_test(ktranscriptcleantest.cpp ../src/i18n/ktranscript.cpp ../src/i18n/common_helpers.cpp
    TEST_NAME ki18n-ktranscriptcleantest
    LINK_LIBRARIES Qt6::Qml Qt6::Test Qt6::Concurrent KF6::I18n
)
target_compile_definitions(ki18n-ktranscriptcleantest PRIVATE "KTRANSCRIPT_TESTBUILD")
target_include_directories(ki18n-ktranscriptcleantest PRIVATE ..)
endif()

add_test(ki18n_install ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMAKE_CURRENT_SOURCE_DIR}/ki18n_install"
    "${CMAKE_CURRENT_BINARY_DIR}/ki18n_install"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-target install
    --build-options
        "-DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/ki18n_install/destdir"
        "-DKF6I18n_DIR=${CMAKE_BINARY_DIR}/cmake"
    --test-command ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/ki18n_install/test.cmake")

ecm_add_tests(
    kcatalogtest.cpp
    kcountrytest.cpp
    kcountrysubdivisiontest.cpp
    ktimezonetest.cpp
    LINK_LIBRARIES KF6::I18nLocaleData Qt6::Test
)

add_executable(qtcatalogtest qtcatalogtest.cpp)
target_link_libraries(qtcatalogtest PRIVATE KF6::I18n Qt6::Test)
function(add_qt_catalog_test name langs expected)
    add_test(NAME qtcatalog-${name} COMMAND qtcatalogtest "QShortcut" "Cancel" "${expected}")
    set_tests_properties(qtcatalog-${name} PROPERTIES
        ENVIRONMENT "LANGUAGE=${langs}"
    )
endfunction()

add_qt_catalog_test(germany "de_DE" "Abbrechen")
# there's no country specifc translations for GB or AT, we expect the generic variants for those,
# not the next language in the list
add_qt_catalog_test(uk-with-fallback "en_GB:ca_ES" "Cancel")
add_qt_catalog_test(austria-with-fallback "de_AT:ca_ES" "Abbrechen")
# same as above, but with multiple country-specific variants
add_qt_catalog_test(au-uk-with-fallback "en_AU:en_GB:ca_ES" "Cancel")
add_qt_catalog_test(austria-switzerland-with-fallback "de_AT:de_CH:ca_ES" "Abbrechen")
# there's no af translation, so we expect the next fallback here
add_qt_catalog_test(afrikaans-with-fallback "af_SA:fr_FR" "Annuler")
