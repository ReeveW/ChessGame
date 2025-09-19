cmake_minimum_required(VERSION 3.10)
project(ChessGame)

set(CMAKE_CXX_STANDARD 17)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(ChessGame
        main.cpp
        main.h
        Board.cpp
        Board.h
        Game.cpp
        Game.h)

# Link SFML libraries
target_link_libraries(ChessGame sfml-graphics sfml-window sfml-system)

# --- Copy assets folder into build directory ---
file(COPY ${CMAKE_SOURCE_DIR}/images DESTINATION ${CMAKE_BINARY_DIR})
