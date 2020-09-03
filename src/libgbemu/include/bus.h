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

    enum JoypadButton : unsigned int
    {
        Down   = 1 << 7,
        Up     = 1 << 6,
        Left   = 1 << 5,
        Right  = 1 << 4,
        Start  = 1 << 3,
        Select = 1 << 2,
        B      = 1 << 1,
        A      = 1 << 0
    };

    enum class AccessType
    {
        Emulated,
        Direct
    };

    // Interconnect between the CPU and devices.
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
                  const AccessType type = AccessType::Emulated) noexcept -> uint8_t;

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

        union
        {
            struct
            {
                unsigned int        : 4;
                unsigned int dpad   : 1;
                unsigned int button : 1;
                unsigned int        : 1;
            };
            uint8_t byte;
        } joypad;

        // The current state of the joypad.
        uint8_t joypad_state;

        // $FF0F - IF - Interrupt Flag (R/W)
        // $FFFF - IE - Interrupt Enable (R/W)
        union
        {
            struct
            {
                unsigned int vblank   : 1;
                unsigned int lcd_stat : 1;
                unsigned int timer    : 1;
                unsigned int serial   : 1;
                unsigned int joypad   : 1;
                unsigned int          : 3;
            };
            uint8_t byte;
        } interrupt_enable, interrupt_flag;

        // [$FF80 - $FFFE] - High RAM (HRAM)
        std::array<uint8_t, 127> hram;

        // APU (audio processing unit) hardware instance
        APU apu;

        // PPU (picture processing unit) hardware instance
        PPU ppu;

        // Timer hardware instance
        Timer timer;

        // The number of cycles taken by the current step.
        unsigned int cycles;

    private:
        // If, and only if the boot ROM is present, this boolean determines
        // whether or not memory reads from addresses <0x100 will be from the
        // boot ROM (false) or the cartridge (true).
        bool boot_rom_disabled;

        // Boot ROM data
        std::vector<uint8_t> m_boot_rom;

        // Game Boy cartridge
        std::shared_ptr<Cartridge> m_cart;
    };
}