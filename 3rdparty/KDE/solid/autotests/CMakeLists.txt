include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Concurrent Test)

########### fakehardwaretest ###############

if (BUILD_DEVICE_BACKEND_fakehw)
    ecm_add_test(fakehardwaretest.cpp LINK_LIBRARIES KF6Solid_static Qt6::Xml Qt6::Test)
    # QT5 TODO Port solid to QFINDTESTDATA
    target_compile_definitions(fakehardwaretest PRIVATE SOLID_STATIC_DEFINE=1 TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/devices/backends/fakehw/fakecomputer.xml")
endif()

########### solidhwtest ###############

if (BUILD_DEVICE_BACKEND_fakehw)
    ecm_add_test(solidhwtest.cpp LINK_LIBRARIES Qt6::Xml Qt6::Test ${LIBS} KF6Solid_static)
    target_compile_definitions(solidhwtest PRIVATE SOLID_STATIC_DEFINE=1 FAKE_COMPUTER_XML="${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/devices/backends/fakehw/fakecomputer.xml")
    target_include_directories(solidhwtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/devices/backends/fakehw)
endif()

########### solidmttest ###############

ecm_add_test(solidmttest.cpp LINK_LIBRARIES Qt6::Xml Qt6::Test ${LIBS} KF6Solid_static Qt6::Concurrent)
target_compile_definitions(solidmttest PRIVATE SOLID_STATIC_DEFINE=1)

########### predicatetest ###############

ecm_add_test(predicatetest.cpp LINK_LIBRARIES Qt6::Xml Qt6::Test ${LIBS} KF6Solid_static Qt6::Concurrent)
target_compile_definitions(predicatetest PRIVATE SOLID_STATIC_DEFINE=1)
