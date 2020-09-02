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

// Required for the `GameBoy::MBC3` class.
#include "mbc3.h"

using namespace GameBoy;

MBC3Cartridge::MBC3Cartridge(const std::vector<uint8_t>& data) noexcept :
Cartridge(data)
{ }

// Returns data from the cartridge referenced by memory address
// `address`.
auto MBC3Cartridge::read(const uint16_t address) noexcept -> uint8_t
{
    switch (address >> 12)
    {
        // [$0000 - $3FFF]: ROM bank $00 (R)
        case 0x0 ... 0x3:
            return m_data[address];

        // [$4000 - $7FFF]: ROM bank $01-$7F (R)
        //
        // Same as for MBC1, except that accessing banks $20, $40, and $60 is 
        // supported now.
        case 0x4 ... 0x7:
            return m_data[(address - 0x4000) + (rom_bank * 0x4000)];

        // [$A000 - $BFFF]: RAM bank $00-$03, if any (R/W)
        // [$A000 - $BFFF]: RTC Register $08-$0C (R/W)
        //
        // Depending on the current bank number/RTC register selection, this
        // memory space is used to access an 8KB external RAM bank, or a single
        // RTC register.
        case 0xA ... 0xB:
            return ram[(address - 0xA000) + (ram_bank * 0x2000)];

        default:
            return address;
    }
}

// Updates the memory bank controller configuration `address` to
// `value`.
auto MBC3Cartridge::write(const uint16_t address,
                          const uint8_t value) noexcept -> void
{
    switch (address >> 12)
    {
        // [$0000 - $1FFF]: RAM and Timer Enable (W)
        //
        // Mostly the same as for MBC1, a value of $0A will enable reading and
        // writing to external RAM and to the RTC registers. A value of $00
        // will disable either.
        case 0x0 ... 0x1:
            return;

        // [$2000 - $3FFF]: ROM bank number (W)
        //
        // Same as MBC1, except that the whole 7 bits of the RAM bank number
        // are written directly to this address. As for the MBC1, writing a
        // value of $00, will select Bank $01 instead. All other values $01-$7F
        // select the corresponding ROM banks.
        case 0x2 ... 0x3:
            rom_bank = value;
            return;

        // [$4000 - $5FFF]: RAM bank number or RTC register select (W)
        //
        // As for the MBC1s RAM banking mode, writing a value in range for
        // $00-$03 maps the corresponding external RAM Bank if any into memory
        // at [$A000 - $BFFF].
        //
        // When writing a value of $08-$0C, this will map the corresponding RTC
        // register into memory at [$A000 - $BFFF]. That register could then be
        // read/written by accessing any address in that area, typically that
        // is done by using address A000.
        case 0x4 ... 0x5:
            ram_bank = value;
            return;

        // [$6000 - $7FFF]: Latch Clock Data (W)
        //
        // When writing $00, and then $01 to this register, the current time
        // becomes latched into the RTC registers. The latched data will not
        // change until it becomes latched again, by repeating the write
        // $00->$01 procedure.
        //
        // This is supposed for reading from the RTC registers. It is proof to
        // read the latched(frozen) time from the RTC registers, while the
        // clock itself continues to tick in background.
        case 0x6 ... 0x7:
            return;

        // [$A000 - $BFFF]: RAM bank $00-$03, if any (R/W)
        // [$A000 - $BFFF]: RTC Register $08-$0C (R/W)
        //
        // Depending on the current bank number/RTC register selection, this
        // memory space is used to access an 8KB external RAM bank, or a single
        // RTC register.
        case 0xA ... 0xB:
            ram[(address - 0xA000) + (ram_bank * 0x2000)] = value;
            return;
    }
}