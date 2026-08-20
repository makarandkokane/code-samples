cmake_minimum_required(VERSION 3.21)
project(observer-mvc VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(observer-mvc
    src/main.cpp
    src/ItemModel.h
    src/ItemModel.cpp
    src/BarChartWidget.h
    src/BarChartWidget.cpp
    src/SummaryPanel.h
    src/SummaryPanel.cpp
    src/ValueDelegate.h
    src/ValueDelegate.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(observer-mvc PRIVATE Qt6::Widgets)

set_target_properties(observer-mvc PROPERTIES WIN32_EXECUTABLE ON)
