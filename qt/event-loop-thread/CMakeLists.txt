cmake_minimum_required(VERSION 3.21)
project(event-loop-thread VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(event-loop-thread
    src/main.cpp
    src/BoxOffice.h
    src/BoxOffice.cpp
    src/BookingAgent.h
    src/BookingAgent.cpp
    src/StadiumView.h
    src/StadiumView.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(event-loop-thread PRIVATE Qt6::Widgets)

set_target_properties(event-loop-thread PROPERTIES WIN32_EXECUTABLE ON)
