cmake_minimum_required(VERSION 3.21)
project(LidSleepTray VERSION 1.0.0 LANGUAGES CXX)

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
)
target_compile_definitions(LidSleepTray PRIVATE APP_VERSION="${PROJECT_VERSION}")
target_link_libraries(LidSleepTray PRIVATE Qt6::Widgets)
if(WIN32)
    target_link_libraries(LidSleepTray PRIVATE powrprof)
endif()
