cmake_minimum_required(VERSION 3.16)
project(window-tree CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(window-tree
    main.cpp
    Widget.cpp
    Widget.h
    Window.cpp
    Window.h
    Panel.cpp
    Panel.h
    Button.cpp
    Button.h
    TextBox.cpp
    TextBox.h
    Screen.cpp
    Screen.h
)
