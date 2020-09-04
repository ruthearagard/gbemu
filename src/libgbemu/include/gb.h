// Copyright 2020 Michael Rodriguez
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

// XXX: NONSTANDARD
//
// "If #pragma once is seen when scanning a header file, that file will never
// be read again, no matter what. It is a less-portable alternative to using
// `#ifndef` to guard the contents of header files against multiple
// inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

#include "bus.h"
#include "cpu.h"

namespace GameBoy
{
    /// @brief Defines a Game Boy system.
    class System
    {
    public:
        /// @brief Initializes a Game Boy system.
        System() noexcept;

        /// @brief Resets the system to the startup state.
        auto reset() noexcept -> void;

        /// @brief Presses a button on the virtual joypad.
        /// @param button The button to press.
        auto press_button(const JoypadButton button) noexcept -> void;

        /// @brief Releases a button on the virtual joypad.
        /// @param button The button to release.
        auto release_button(const JoypadButton button) noexcept -> void;

        /// @brief Generates a cartridge.
        /// 
        /// This function will throw an `std::runtime_error` under the
        /// following circumstances:
        /// 
        /// * The header checksum verification failed, or;
        /// * The ROM requires a memory bank controller we don't support.
        /// 
        /// @param cart_data The data to use to generate the Cartridge
        /// instance.
        /// @return The generated cartridge.
        auto cart(const std::vector<uint8_t>& cart_data) ->
        std::shared_ptr<Cartridge>;

        /// @brief Sets the boot ROM data.
        /// @param data If `data` is not empty, a boot ROM is considered to be
        /// present. Otherwise, boot ROM functionality will be disabled.
        auto boot_rom(const std::vector<uint8_t>& data) noexcept -> void;

        /// @brief Executes one full system step.
        /// @return The number of T-cycles taken by the current step.
        auto step() noexcept -> unsigned int;

        /// @brief System bus instance
        SystemBus bus;

        /// @brief Sharp SM83 CPU interpreter instance
        CPU cpu;
    };
}