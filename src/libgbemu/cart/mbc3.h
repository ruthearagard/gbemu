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

#include <array>
#include "../include/cart.h"

namespace GameBoy
{
    /// @brief Defines an MBC3 based cartridge.
    class MBC3Cartridge final : public Cartridge
    {
    public:
        explicit MBC3Cartridge(const std::vector<uint8_t>& data) noexcept;

        /// @brief Returns a byte from the cartridge.
        /// @param address The memory address to read from.
        /// @return The byte from the cartridge.
        auto read(const uint16_t address) noexcept -> uint8_t;

        /// @brief Updates the memory bank controller configuration.
        /// @param address The configuration area to update.
        /// @param value The value to update the area with.
        auto write(const uint16_t address,
                   const uint8_t value) noexcept -> void;

    private:
        /// @brief 32KB RAM
        std::array<uint8_t, 32768> ram;

        /// @brief The current ROM bank.
        uint8_t rom_bank;

        /// @brief The current RAM bank.
        uint8_t ram_bank;
    };
}