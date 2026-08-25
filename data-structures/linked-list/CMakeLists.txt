cmake_minimum_required(VERSION 3.16)
project(linked-list C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(linked-list
    main.c
    LinkedList.c
    LinkedList.h
)
