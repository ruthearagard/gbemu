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

// Required for `std::array<>`.
#include <array>

// Required for fixed-width integer types (e.g. `uint8_t`).
#include <cstdint>

// Required for `std::unique_ptr<>`.
#include <memory>

// Required for `std::vector<>`.
#include <vector>

// Required for the `GameBoy::APU` class.
#include "apu.h"

// Required for the `GameBoy::PPU` class.
#include "ppu.h"

// Required for the `GameBoy::Timer` class.
#include "timer.h"

namespace GameBoy
{
    // Forward declaration
    class Cartridge;

    enum Interrupt : unsigned int
    {
        VBlankInterrupt = 1 << 0,
        TimerInterrupt  = 1 << 2
    };

    enum class AccessType
    {
        Emulated,
        Direct
    };

    class SystemBus
    {
    public:
        SystemBus() noexcept;

        // Sets the current cartridge to `cart`.
        auto cart(const std::shared_ptr<Cartridge>& cart) noexcept -> void;

        // Sets the current boot ROM to `data`.
        auto boot_rom(const std::vector<uint8_t>& data) noexcept -> void;

        // Resets the hardware to the startup state.
        auto reset() noexcept -> void;

        // Advances the hardware by 1 m-cycle.
        auto step() noexcept -> void;

        // Returns a byte from memory referenced by memory address `address`.
        // This function incurs 1 m-cycle (or 4 T-cycles).
        auto read(const uint16_t address,
                  const AccessType access_type = AccessType::Emulated) noexcept -> uint8_t;

        // Stores a byte `data` into memory referenced by memory address
        // `address`.
        //
        // This function incurs 1 m-cycle (or 4 T-cycles).
        auto write(const uint16_t address,
                   const uint8_t data) noexcept -> void;

        // Signals an interrupt `interrupt`.
        auto signal_interrupt(const Interrupt interrupt) noexcept -> void;

        // [$C000 - $CFFF] - 4KB Work RAM Bank 0 (WRAM)
        std::array<uint8_t, 4096> wram;

        // [$D000 - $DFFF] - 4KB Work RAM Bank 1 (WRAM)
        std::array<uint8_t, 4096> wram1;

        // $FF0F - IF - Interrupt Flag (R/W)
        uint8_t interrupt_flag;

        // [$FF80 - $FFFE] - High RAM (HRAM)
        std::array<uint8_t, 127> hram;

        // $FFFF - IE - Interrupt Enable (R/W)
        uint8_t interrupt_enable;

        // APU (audio processing unit) hardware instance
        APU apu;

        // PPU (picture processing unit) hardware instance
        PPU ppu;

        // Timer hardware instance
        Timer timer;

        unsigned int cycles = 0;

    private:
        bool boot_rom_disabled;
        std::vector<uint8_t> m_boot_rom;
        std::shared_ptr<Cartridge> m_cart;
    };
}