cmake_minimum_required(VERSION 3.10)

project(Chip8Emulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(chip8_emu 
    main.cpp 
    chip8.cpp
)

target_link_libraries(chip8_emu PRIVATE SDL2::SDL2)
