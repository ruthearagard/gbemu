# Copyright 2020 Michael Rodriguez
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

# Using the first version of CMake released in 2019 appears to be reasonable.
#
# FATAL_ERROR is specified so that CMake versions <=2.4 fail with an error
# instead of just a warning.
cmake_minimum_required(VERSION 3.13.3 FATAL_ERROR)

project(gbemu
        VERSION 1.0.0.0
        DESCRIPTION "Game Boy emulator"
        HOMEPAGE_URL "https://github.com/ruthearagard/gbemu"
        LANGUAGES CXX)

# We only support gcc or clang, let's check to see if we're using either or.
# Using the first version of either released in 2019 appears to be reasonable.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION LESS "8.0.0")
        message(FATAL_ERROR "This project requires clang >=8.00 or gcc >=8.3.")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION LESS "8.3")
        message(FATAL_ERROR "This project requires clang >=8.00 or gcc >=8.3.")
    endif()
endif()

add_subdirectory(src)