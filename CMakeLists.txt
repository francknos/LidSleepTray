cmake_minimum_required(VERSION 3.21)
project(LidSleepTray VERSION 1.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(LidSleepTray WIN32
    src/main.cpp
    src/PowerManager.cpp
    src/PowerManager.h
    src/SettingsPopup.cpp
    src/SettingsPopup.h
    src/TrayApp.cpp
    src/TrayApp.h
    src/ToggleSwitch.cpp
    src/ToggleSwitch.h
)
target_compile_definitions(LidSleepTray PRIVATE APP_VERSION="${PROJECT_VERSION}")
target_link_libraries(LidSleepTray PRIVATE Qt6::Widgets)
if(WIN32)
    enable_language(RC)
    configure_file(assets/app.rc.in "${CMAKE_CURRENT_BINARY_DIR}/app.rc" @ONLY)
    target_sources(LidSleepTray PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
    target_link_libraries(LidSleepTray PRIVATE powrprof)
endif()

option(LIDSLEEPTRAY_BUILD_TESTS "Build UI tests" OFF)
if(LIDSLEEPTRAY_BUILD_TESTS)
    enable_testing()
    add_executable(SettingsPopupTest
        tests/SettingsPopupTest.cpp
        src/SettingsPopup.cpp
        src/SettingsPopup.h
        src/ToggleSwitch.cpp
        src/ToggleSwitch.h
    )
    target_include_directories(SettingsPopupTest PRIVATE src)
    target_compile_definitions(SettingsPopupTest PRIVATE APP_VERSION="${PROJECT_VERSION}")
    target_link_libraries(SettingsPopupTest PRIVATE Qt6::Widgets)
    add_test(NAME SettingsPopupTest COMMAND SettingsPopupTest)
    set_tests_properties(SettingsPopupTest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
