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

find_package(fmt CONFIG REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# List of source files (*.cpp) and header files (*.h).
#
# We don't use file(GLOB ...) to handle these files, as it is not recommended:
#
# "We do not recommend using GLOB to collect a list of source files from your
# source tree. If no CMakeLists.txt file changes when a source is added or
# removed then the generated build system cannot know when to ask CMake to
# regenerate. The CONFIGURE_DEPENDS flag may not work reliably on all
# generators, or if a new generator is added in the future that cannot support
# it, projects using it will be stuck. Even if CONFIGURE_DEPENDS works
# reliably, there is still a cost to perform the check on every rebuild."
#
# Source (heh): https://cmake.org/cmake/help/v3.13/command/file.html#filesystem
set(SRCS disasm.cpp emulator.cpp gbemu.cpp main.cpp main_window.cpp opengl.cpp)
set(HDRS disasm.h emulator.h gbemu.h main_window.h opengl.h)

add_executable(gbemu_main ${SRCS} ${HDRS})

set_target_properties(gbemu_main PROPERTIES
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS ON)

target_link_libraries(gbemu_main PRIVATE gbemu fmt::fmt
                                               fmt::fmt-header-only
                                               Qt5::Core
                                               Qt5::Gui
                                               Qt5::Multimedia
                                               Qt5::Widgets)

target_compile_options(gbemu_main PRIVATE -Wall -Wextra)
