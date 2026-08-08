cmake_minimum_required(VERSION 3.16)
project(LightJSEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Release build if the user didn't pick one, so plain
# `cmake .. && make` doesn't silently produce an unoptimized binary.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(light-js
    main.cpp
)

# All the engine logic lives in headers (Lexer.hpp, Compiler.hpp, Chunk.hpp,
# Value.hpp, VM.hpp, NativeBindings.hpp) included directly by main.cpp, so
# there's only one translation unit. If you split into .cpp
# files later, add them to the add_executable() list above.
target_include_directories(light-js PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(light-js PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-fsanitize=address,undefined -g>
)
target_link_options(light-js PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)
