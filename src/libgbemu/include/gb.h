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
// ‘#ifndef’ to guard the contents of header files against multiple inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

// Required for the `GameBoy::SystemBus` class.
#include "bus.h"

// Required for the `GameBoy::Cartridge` class.
#include "cart.h"

// Required for the `GameBoy::CPU` class.
#include "cpu.h"

namespace GameBoy
{
    // Defines a Game Boy system.
    class System
    {
    public:
        System() noexcept;

        // Resets the system to the startup state.
        auto reset() noexcept -> void;

        // Sets the current cartridge to `cart`.
        auto cart(const std::shared_ptr<Cartridge>& c) noexcept -> void;

        // Sets the current boot ROM data to `boot_rom`.
        auto boot_rom(const std::vector<uint8_t>& data) noexcept -> void;

        // Executes one full step and returns the number of cycles taken.
        auto step() noexcept -> unsigned int;

        // System bus instance
        SystemBus bus;

        // Sharp SM83 CPU interpreter instance
        CPU cpu;
    };
}