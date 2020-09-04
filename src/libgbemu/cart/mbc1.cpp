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

#include "mbc1.h"

using namespace GameBoy;

MBC1Cartridge::MBC1Cartridge(const std::vector<uint8_t>& data) noexcept :
Cartridge(data)
{
    banking_mode = BankingMode::ROM;

    rom_bank.byte = 0x01;
    ram_bank      = 0x00;
    
    ram = { };
}

/// @brief Returns a byte from the cartridge.
/// @param address The memory address to read from.
/// @return The byte from the cartridge.
auto MBC1Cartridge::read(const uint16_t address) noexcept -> uint8_t
{
    switch (address >> 12)
    {
        // [$0000 - $3FFF]: ROM bank $00 (R)
        //
        // This area always contains the first 16KB of the cartridge ROM.
        case 0x0 ... 0x3:
            return m_data[address];

        // [$4000 - $7FFF]: ROM bank $01-7F (R)
        //
        // This area may contain any of the further 16KB banks of the ROM,
        // allowing to address up to 125 ROM Banks (almost 2 MB). Bank numbers
        // $20, $40, and $60 cannot be used, resulting in the odd amount of 125
        // banks.
        case 0x4 ... 0x7:
            return m_data[(address - 0x4000) + (rom_bank.byte * 0x4000)];

        // [$A000 - $BFFF]: RAM bank $00-$03, if any (R/W)
        //
        // This area is used to address external RAM in the cartridge, if any.
        // External RAM is often battery buffered, allowing to store game
        // positions or high score tables, even if the GameBoy is turned off,
        // or if the cartridge is removed from the GameBoy.
        case 0xA ... 0xB:
            return ram[(address - 0xA000) + (ram_bank * 0x2000)];

        default:
            return 0;
    }
}

/// @brief Updates the memory bank controller configuration.
/// @param address The configuration area to update.
/// @param value The value to update the area with.
auto MBC1Cartridge::write(const uint16_t address,
                          const uint8_t value) noexcept -> void
{
    switch (address >> 12)
    {
        // [$0000 - $1FFF]: External RAM Control (W)
        //
        // Before external RAM can be read or written, it must be enabled by
        // writing to this address space. Practically any value with $0A in the
        // lower 4 bits enables RAM, and any other value disables RAM.
        case 0x0 ... 0x1:
            ram_enabled = (value & 0x0F) == 0x0A;
            return;

        // [$2000 - $3FFF]: ROM bank number (W)
        //
        // Writing to this address space selects the lower 5 bits of the ROM
        // Bank Number (in range $01-$1F). When $00 is written, the MBC
        // translates that to bank $01 also. That doesn't harm so far, because
        // ROM Bank $00 can be always directly accessed by reading from
        // $0000 - $3FFF.
        //
        // But when using the register below to specify the upper ROM Bank
        // bits, the same happens for Bank $20, $40, and $60. Any attempt to
        // address these ROM Banks will select Bank $21, $41, and $61 instead.
        case 0x2 ... 0x3:
            rom_bank.lo = value & 0x1F;
            return;

        // [$4000 - $5FFF]: RAM bank number or upper bits of ROM bank number (W)
        //
        // This 2-bit register can be used to select a RAM bank in range from
        // $00-$03, or to specify the upper two bits (Bit 5-6) of the ROM bank
        // number, depending on the current ROM/RAM Mode.
        case 0x4 ... 0x5:
            if (banking_mode == BankingMode::RAM)
            {
                ram_bank = value;
            }
            else
            {
                rom_bank.hi = value;
            }
            return;

        // [$6000 - $7FFF]: ROM/RAM Mode Select (W)
        //
        // This 1-bit register selects whether the two bits of the above
        // register should be used as upper two bits of the ROM bank, or as RAM
        // bank number.
        //
        // $00 = ROM Banking Mode (up to 8KB RAM, 2MB ROM) (default)
        // $01 = RAM Banking Mode (up to 32KB RAM, 512KB ROM)
        //
        // The program may freely switch between both modes, the only
        // limitation is that only RAM bank $00 can be used during Mode 0, and
        // only ROM Banks $00-$1F can be used during Mode 1.
        case 0x6 ... 0x7:
            banking_mode = value == 0x00 ? BankingMode::ROM : BankingMode::RAM;
            return;
    }
}