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

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include "apu.h"
#include "ppu.h"
#include "scheduler.h"
#include "timer.h"

namespace GameBoy
{
    class Cartridge;

    /// @brief Types of interrupts possible.
    enum Interrupt : unsigned int
    {
        VBlankInterrupt = 1 << 0,
        TimerInterrupt  = 1 << 2
    };

    /// @brief List of joypad bits.
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

    /// @brief Memory access types.
    enum class AccessType
    {
        /// @brief The memory function will step the hardware by 1 m-cycle.
        Emulated,

        /// @brief The memory function will NOT step the hardware by 1 m-cycle.
        /// 
        /// This is useful for memory inspection.
        Direct
    };

    /// @brief Defines the interconnect between the CPU, memory, and devices.
    class SystemBus final
    {
    public:
        SystemBus() noexcept;

        /// @brief Sets the current cartridge.
        /// @param cart The cartridge to set.
        auto cart(const std::shared_ptr<Cartridge>& cart) noexcept -> void;

        /// @brief Sets the current boot ROM.
        /// @param data The boot ROM data. If this is empty, then the boot ROM
        /// will be disabled, if previously enabled.
        auto boot_rom(const std::vector<uint8_t>& data) noexcept -> void;

        /// @brief Resets the devices to their startup state and clears all
        /// memory.
        auto reset() noexcept -> void;

        /// @brief Advances the devices by 1 m-cycle.
        auto step() noexcept -> void;

        /// @brief Request an interrupt.
        /// @param interrupt The interrupt to request.
        auto irq(const Interrupt interrupt) noexcept -> void;

        /// @brief Returns a byte from memory.
        /// @param address The address to read from.
        /// @param type
        /// 
        /// `AccessType::Emulated` (default): Steps the devices by 1 m-cycle.
        /// `AccessType::Direct`: Does not step the devices.
        /// @return The byte from memory.
        auto read(const uint16_t address,
                  const AccessType type = AccessType::Emulated) noexcept -> uint8_t;

        /// @brief Stores a byte into memory.
        /// @param address The address to write to.
        /// @param data The data to store at the address.
        auto write(const uint16_t address,
                   const uint8_t data) noexcept -> void;

        /// @brief [$C000 - $DFFF]: 4KB Work RAM Bank 0-1 (WRAM)
        std::array<uint8_t, 8192> wram;

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

        /// @brief [$FF80 - $FFFE]: High RAM (HRAM)
        std::array<uint8_t, 127> hram;

        /// @brief APU (audio processing unit) device instance
        APU apu;

        /// @brief PPU (picture processing unit) device instance
        PPU ppu;

        /// @brief Scheduler instance
        Scheduler sched;

        /// @brief Timer device instance
        Timer timer;

        // The number of cycles taken by the current step.
        unsigned int cycles;

    private:
        /// @brief Is the boot ROM disabled?
        /// 
        /// This only matters if the user has requested to use a boot ROM. It
        /// governs whether or not reads from memory addresses <$0100 will
        /// return data from the boot ROM (false) or from the game cartridge
        /// (true).
        bool boot_rom_disabled;

        /// @brief Boot ROM data, if any.
        std::vector<uint8_t> m_boot_rom;

        /// @brief The current Game Boy cartridge.
        std::shared_ptr<Cartridge> m_cart;
    };
}
