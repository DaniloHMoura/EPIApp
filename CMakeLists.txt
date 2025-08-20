cmake_minimum_required(VERSION 3.16)
project(EPIApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Charts)
qt_standard_project_setup()

add_executable(EPIApp
    main.cpp
    DatabaseManager.cpp DatabaseManager.h
    LoginDialog.cpp LoginDialog.h
    EPIApp.cpp EPIApp.h
)

target_link_libraries(EPIApp PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Charts)

set_target_properties(EPIApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
