cmake_minimum_required(VERSION 3.16)
project(LightJSEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Single-config generators (Makefiles, Ninja) pick their build type via
# CMAKE_BUILD_TYPE at configure time; default to Release if unset so a
# plain `cmake .. && make` doesn't silently produce an unoptimized build.
#
# Multi-config generators (Visual Studio, Xcode) ignore CMAKE_BUILD_TYPE
# entirely and already ship their own default config list (typically
# Debug;Release;RelWithDebInfo;MinSizeRel) — no need to set anything for
# them here. The config is chosen at build time (`cmake --build . --config
# Release`) or via the IDE's dropdown, which defaults to Debug; pass
# --config Release explicitly when you want the optimized build.
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

add_executable(light-js
    main.cpp
)

# All the engine logic lives in headers (Lexer.hpp, Compiler.hpp, Chunk.hpp,
# Value.hpp, VM.hpp, NativeBindings.hpp) included directly by main.cpp, so
# there's only one translation unit. If you split implementations into .cpp
# files later, add them to the add_executable() list above.
target_include_directories(light-js PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    # /W4 is MSVC's rough equivalent of -Wall -Wextra.
    # MSVC's sanitizer support (/fsanitize=address) is real but newer,
    # more limited in coverage than ASan+UBSan on GCC/Clang, and not
    # every MSVC toolchain has it installed — so it's left off by
    # default rather than assumed present. Enable manually if your
    # toolchain supports it: target_compile_options(light-js PRIVATE
    # $<$<CONFIG:Debug>:/fsanitize=address>)
    target_compile_options(light-js PRIVATE
        /W4
        $<$<CONFIG:Debug>:/Zi>
    )
else()
    # GCC and Clang both understand this flag set.
    target_compile_options(light-js PRIVATE
        -Wall
        -Wextra
        $<$<CONFIG:Debug>:-fsanitize=address,undefined -g>
    )
    target_link_options(light-js PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
endif()
