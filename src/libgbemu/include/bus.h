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

// Required for fixed-width integer types (e.g. `uint8_t`).
#include <cstdint>

// Required for `std::unique_ptr<>`.
#include <memory>

namespace GameBoy
{
    // Forward declaration
    class Cartridge;

    class SystemBus
    {
    public:
        // Sets the current cartridge to `cart`.
        auto cart(const std::shared_ptr<Cartridge>& cart) noexcept -> void;

        // Returns a byte from memory referenced by memory address `address`.
        // This function incurs 1 m-cycle (or 4 T-cycles).
        auto read(const uint16_t address) const noexcept -> uint8_t;

        // Stores a byte `data` into memory referenced by memory address
        // `address`.
        //
        // This function incurs 1 m-cycle (or 4 T-cycles).
        auto write(const uint16_t address,
                   const uint8_t data) noexcept -> void;

    private:
        std::shared_ptr<Cartridge> m_cart;
    };
}