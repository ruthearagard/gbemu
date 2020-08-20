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

        // [$C000 - $CFFF] - 4KB Work RAM Bank 0 (WRAM)
        case 0xC:
            return wram[address - 0xC000];

        // [$D000 - $DFFF] - 4KB Work RAM Bank 1 (WRAM)
        case 0xD:
            return wram1[address - 0xD000];

        case 0xF:
            switch (address & 0x0FFF)
            {
                // $FF44 - LY - LCDC Y-Coordinate (R)
                case 0xF44:
                    return ppu.LY;

                default:
                    __debugbreak();
                    return 0xFF;
            }

        default:
            __debugbreak();
            return 0xFF;
    }
}

// Stores a byte `data` into memory referenced by memory address `address`.
//
// This function incurs 1 m-cycle (or 4 T-cycles).
auto SystemBus::write(const uint16_t address,
                      const uint8_t data) noexcept -> void
{
    switch (address >> 12)
    {
        // [$C000 - $CFFF] - 4KB Work RAM Bank 0 (WRAM)
        case 0xC:
            wram[address - 0xC000] = data;
            return;

        // [$D000 - $DFFF] - 4KB Work RAM Bank 1 (WRAM)
        case 0xD:
            wram1[address - 0xD000] = data;
            return;

        case 0xF:
            switch (address & 0x0FFF)
            {
                // $FF07 - TAC - Timer Control (R/W)
                case 0xF07:
                    timer.TAC = data;
                    return;

                // $FF0F - IF - Interrupt Flag (R/W)
                case 0xF0F:
                    interrupt_flag = data;
                    return;

                // $FF24 - NR50 - Channel control / ON-OFF / Volume (R/W)
                case 0xF24:
                    apu.NR50 = data;
                    return;

                // $FF25 - NR51 - Selection of Sound output terminal (R/W)
                case 0xF25:
                    apu.NR51 = data;
                    return;

                // $FF26 - NR52 - Sound on/off
                case 0xF26:
                    apu.NR52 = data;
                    return;

                // [$FF80 - $FFFE] - High RAM (HRAM)
                case 0xF80 ... 0xFFE:
                    hram[address - 0xFF80] = data;
                    return;

                // $FFFF - IE - Interrupt Enable (R/W)
                case 0xFFF:
                    interrupt_enable = data;
                    return;

                default:
                    __debugbreak();
                    return;
            }

        default:
            __debugbreak();
            return;
    }
}