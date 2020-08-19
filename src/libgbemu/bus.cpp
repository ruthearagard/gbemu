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

// Required for the `GameBoy::SystemBus` class.
#include "bus.h"

// Required for the `GameBoy::Cartridge` class.
#include "cart.h"

using namespace GameBoy;

// Sets the current cartridge to `cart`.
auto SystemBus::cart(const std::shared_ptr<Cartridge>& cart) noexcept -> void
{
    m_cart = cart;
}

// Returns a byte from memory referenced by memory address `address`.
// This function incurs 1 m-cycle (or 4 T-cycles).
auto SystemBus::read(const uint16_t address) const noexcept -> uint8_t
{
    switch (address >> 12)
    {
        // [$0000 - $3FFF]: 16KB ROM Bank 0 (in cartridge, fixed at bank 0)
        case 0x0 ... 0x3:
            return m_cart->read(address);

        // [$4000 - $7FFF]: 16KB ROM Bank 1..N
        // (in cartridge, switchable bank number)
        case 0x4 ... 0x7:
            return m_cart->read(address);

        default:
            return 0xFF;
    }
}

// Stores a byte `data` into memory referenced by memory address `address`.
//
// This function incurs 1 m-cycle (or 4 T-cycles).
auto SystemBus::write(const uint16_t address,
                      const uint8_t data) noexcept -> void
{
    __debugbreak();
}