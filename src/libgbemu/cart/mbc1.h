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
// �#ifndef� to guard the contents of header files against multiple inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

// Required for `std::array<>`.
#include <array>

// Required for the `GameBoy::Cartridge` class.
#include "../include/cart.h"

namespace GameBoy
{
    class MBC1Cartridge : public Cartridge
    {
    public:
        MBC1Cartridge(const std::vector<uint8_t>& data) noexcept;

        // Returns data from the cartridge referenced by memory address
        // `address`.
        auto read(const uint16_t address) noexcept -> uint8_t;

        // Updates the memory bank controller configuration `address` to
        // `value`.
        auto write(const uint16_t address,
                   const uint8_t value) noexcept -> void;

    private:
        std::array<uint8_t, 32768> ram;

        enum class Mode
        {
            ROM,
            RAM
        };

        union
        {
            struct
            {
                unsigned int lo : 5;
                unsigned int hi : 2;
                unsigned int    : 1;
            };
            uint8_t byte;
        } rom_bank;

        bool ram_enabled;

        uint8_t ram_bank;

        Mode mode;
    };
}