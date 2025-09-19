cmake_minimum_required(VERSION 3.10)
project(SFML_Project)

set(CMAKE_CXX_STANDARD 17)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(SFML_Project main.cpp
        main.h
        Board.cpp
        Board.h
        Game.cpp
        Game.h)

# Link SFML libraries
target_link_libraries(SFML_Project sfml-graphics sfml-window sfml-system)
