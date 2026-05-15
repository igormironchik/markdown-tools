
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)
add_definitions(-DQT_FORCE_ASSERTS=1)

include(ECMAddTests)

if(HAVE_QTDBUS)
    ecm_add_tests(
        kuiserver2jobtrackertest.cpp
        kuiserver2jobtrackertest2.cpp
        LINK_LIBRARIES Qt6::Test Qt6::Widgets KF6::JobWidgets
    )
endif()
