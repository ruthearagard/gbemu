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

SystemBus::SystemBus() noexcept : timer(*this), ppu(*this)
{ }

// Sets the current cartridge to `cart`.
auto SystemBus::cart(const std::shared_ptr<Cartridge>& cart) noexcept -> void
{
    m_cart = cart;
}

// Sets the current boot ROM to `data`.
auto SystemBus::boot_rom(const std::vector<uint8_t>& data) noexcept -> void
{
    m_boot_rom = data;
}

// Resets the hardware to the startup state.
auto SystemBus::reset() noexcept -> void
{
    timer.reset();
    ppu.reset();

    cycles = 0;
    boot_rom_disabled = false;
}

// Advances the hardware by 1 m-cycle.
auto SystemBus::step() noexcept -> void
{
    cycles += 4;

    timer.step();
    ppu.step();
}

// Returns a byte from memory referenced by memory address `address`.
// This function incurs 1 m-cycle (or 4 T-cycles).
auto SystemBus::read(const uint16_t address,
                     const AccessType type) noexcept -> uint8_t
{
    if (type == AccessType::Emulated)
    {
        // Each memory access takes 1 m-cycle.
        step();
    }

    switch (address >> 12)
    {
        case 0x0:
        {
            if ((address < 0x0100)  &&
                !m_boot_rom.empty() &&
                !boot_rom_disabled)
            {
                // [$0000 - $0FFF] - Boot ROM (R)
                return m_boot_rom[address];
            }

            // [$0000 - $3FFF] - ROM Bank 00 (R)
            return m_cart->read(address);
        }

        // [$4000 - $7FFF] - 16KB ROM Bank 01..N
        // (in cartridge, switchable bank number)
        case 0x1 ... 0x7:
            return m_cart->read(address);

        // [$A000 - $BFFF] - 8KB External RAM 
        // (in cartridge, switchable bank, if any)
        case 0xA ... 0xB:
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
                // $FF00 - P1/JOYP - Joypad (R/W)
                case 0xF00:
                    return 0xFF;

                // $FF04 - DIV - Divider Register (R/W)
                case 0xF04:
                    return timer.DIV;

                // $FF05 - TIMA - Timer counter (R/W)
                case 0xF05:
                    return timer.TIMA;

                // $FF0F - IF - Interrupt Flag (R/W)
                case 0xF0F:
                    return interrupt_flag.byte;

                // $FF40 - LCDC - LCD Control (R/W)
                case 0xF40:
                    return ppu.LCDC.byte;

                // $FF42 - SCY - Scroll Y (R/W)
                case 0xF42:
                    return ppu.SCY;

                // $FF44 - LY - LCDC Y-Coordinate (R)
                case 0xF44:
                    return ppu.LY;

                // [$FF80 - $FFFE] - High RAM (HRAM)
                case 0xF80 ... 0xFFE:
                    return hram[address - 0xFF80];

                // $FFFF - IE - Interrupt Enable (R/W)
                case 0xFFF:
                    return interrupt_enable.byte;

                default:
                    //__debugbreak();
                    return 0xFF;
            }

        default:
            //__debugbreak();
            return 0xFF;
    }
}

// Stores a byte `data` into memory referenced by memory address `address`.
//
// This function incurs 1 m-cycle (or 4 T-cycles).
auto SystemBus::write(const uint16_t address,
                      const uint8_t data) noexcept -> void
{
    // Each memory access takes 1 m-cycle.
    step();

    switch (address >> 12)
    {
        // [$0000 - $7FFF]: Cartridge memory bank controller configuration area
        case 0x0 ... 0x7:
            m_cart->write(address, data);
            return;

        // [$8000 - $9FFF] - 8KB Video RAM (VRAM)
        case 0x8 ... 0x9:
            ppu.vram[address - 0x8000] = data;
            return;

        // [$A000 - $BFFF] - 8KB External RAM 
        // (in cartridge, switchable bank, if any)
        case 0xA ... 0xB:
            m_cart->write(address, data);
            return;

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
                // $FF00 - P1 / JOYP - Joypad (R/W)
                case 0xF00:
                    return;

                // $FF01 - SB - Serial transfer data (R/W)
                case 0xF01:
                    printf("%c", data);
                    return;

                // $FF02 - SC - Serial Transfer Control (R/W)
                case 0xF02:
                    return;

                // $FF04 - DIV - Divider Register (R/W)
                case 0xF04:
                    timer.DIV = 0;
                    return;

                // $FF05 - TIMA - Timer counter (R/W)
                case 0xF05:
                    timer.TIMA = data;
                    return;

                // $FF06 - TMA - Timer Modulo (R/W)
                case 0xF06:
                    timer.TMA = data;
                    return;

                // $FF07 - TAC - Timer Control (R/W)
                case 0xF07:
                    timer.TAC.byte = data;
                    return;

                // $FF0F - IF - Interrupt Flag (R/W)
                case 0xF0F:
                    interrupt_flag.byte = data;
                    return;

                // $FF12 - NR12 - Channel 1 Volume Envelope (R/W)
                case 0xF12:
                    return;

                // $FF14 - NR14 - Channel 1 Frequency hi (R/W)
                case 0xF14:
                    return;

                // $FF17 - NR22 - Channel 2 Volume Envelope (R/W)
                case 0xF17:
                    return;

                // $FF21 - NR42 - Channel 4 Volume Envelope (R/W)
                case 0xF21:
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

                // $FF40 - LCDC - LCD Control (R/W)
                case 0xF40:
                    ppu.set_LCDC(data);
                    return;
                
                // $FF41 - STAT - LCDC Status (R/W)
                case 0xF41:
                    ppu.STAT.byte = data;
                    return;

                // $FF42 - SCY - Scroll Y (R/W)
                case 0xF42:
                    ppu.SCY = data;
                    return;

                // $FF43 - SCX - Scroll X (R/W)
                case 0xF43:
                    ppu.SCX = data;
                    return;

                // $FF47 - BGP - BG Palette Data (R/W)
                case 0xF47:
                    ppu.BGP.byte = data;
                    return;

                // $FF48 - OBP0 - Object Palette 0 Data (R/W)
                case 0xF48:
                    return;

                // $FF49 - OBP1 - Object Palette 1 Data (R/W)
                case 0xF49:
                    return;

                // $FF4A - WY - Window Y Position (R/W)
                case 0xF4A:
                    ppu.WY = data;
                    return;

                // $FF4B - WX - Window X Position minus 7 (R/W)
                case 0xF4B:
                    ppu.WX = data;
                    return;

                // $FF50 - Disable boot ROM
                case 0xF50:
                    boot_rom_disabled = true;
                    return;

                // [$FF80 - $FFFE] - High RAM (HRAM)
                case 0xF80 ... 0xFFE:
                    hram[address - 0xFF80] = data;
                    return;

                // $FFFF - IE - Interrupt Enable (R/W)
                case 0xFFF:
                    interrupt_enable.byte = data;
                    return;

                default:
                    //__debugbreak();
                    return;
            }

        default:
            //__debugbreak();
            return;
    }
}

// Signals an interrupt `interrupt`.
auto SystemBus::signal_interrupt(const Interrupt interrupt) noexcept -> void
{
    interrupt_flag.byte |= interrupt;
}