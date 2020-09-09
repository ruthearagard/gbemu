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

#include "bus.h"
#include "cart.h"

using namespace GameBoy;

SystemBus::SystemBus() noexcept : timer(*this), ppu(*this)
{ }

/// @brief Sets the current cartridge.
/// @param cart The cartridge to set.
auto SystemBus::cart(const std::shared_ptr<Cartridge>& cart) noexcept -> void
{
    m_cart = cart;
}

/// @brief Sets the current boot ROM.
/// @param data The boot ROM data. If this is empty, then the boot ROM will be
/// disabled, if previously enabled.
auto SystemBus::boot_rom(const std::vector<uint8_t>& data) noexcept -> void
{
    m_boot_rom = data;
}

/// @brief Resets the devices to their startup state and clears all memory.
auto SystemBus::reset() noexcept -> void
{
    apu.reset();
    ppu.reset();
    timer.reset();

    joypad_state = 0xFF;

    cycles = 0;
    boot_rom_disabled = false;
}

/// @brief Advances the devices by 1 m-cycle.
auto SystemBus::step() noexcept -> void
{
    cycles += 4;

    apu.step();
    ppu.step();
    timer.step();
}

/// @brief Request an interrupt.
/// @param interrupt The interrupt to request.
auto SystemBus::irq(const Interrupt interrupt) noexcept -> void
{
    interrupt_flag.byte |= interrupt;
}

/// @brief Returns a byte from memory.
/// @param address The address to read from.
/// @param type
/// 
/// `AccessType::Emulated` (default): Steps the devices by 1 m-cycle.
/// `AccessType::Direct`: Does not step the devices.
/// @return The byte from memory.
auto SystemBus::read(const uint16_t address,
                     const AccessType type) noexcept -> uint8_t
{
    if (type == AccessType::Emulated)
    {
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
                return m_boot_rom[address];
            }
            return m_cart->read(address);
        }

        case 0x1 ... 0x7: return m_cart->read(address);
        case 0xA ... 0xB: return m_cart->read(address);
        case 0xC ... 0xD: return wram[address - 0xC000];

        case 0xF:
            switch (address & 0x0FFF)
            {
                // $FF00 - P1/JOYP - Joypad (R/W)
                case 0xF00:
                {
                    if (joypad.dpad)
                    {
                        return joypad_state >> 4;
                    }
                    else if (joypad.button)
                    {
                        return joypad_state & 0x0F;
                    }
                    return 0xFF;
                }

                case 0xF04:           return timer.DIV;
                case 0xF05:           return timer.TIMA;
                case 0xF0F:           return interrupt_flag.byte;
                case 0xF10:           return apu.CH1.NR10.byte;
                case 0xF11:           return apu.CH1.NR11.byte;
                case 0xF25:           return apu.NR51.byte;
                case 0xF26:           return apu.NR52.byte;
                case 0xF30 ... 0xF3F: return apu.CH3.ram[address - 0xFF30];
                case 0xF40:           return ppu.get_LCDC();
                case 0xF42:           return ppu.SCY;
                case 0xF44:           return ppu.LY;
                case 0xF48:           return ppu.OBP0.byte;
                case 0xF49:           return ppu.OBP1.byte;
                case 0xF80 ... 0xFFE: return hram[address - 0xFF80];
                case 0xFFF:           return interrupt_enable.byte;

                default:
                    __debugbreak();
                    return 0xFF;
            }

        default:
            __debugbreak();
            return 0xFF;
    }
}

/// @brief Stores a byte into memory.
/// @param address The address to write to.
/// @param data The data to store at the address.
auto SystemBus::write(const uint16_t address,
                      const uint8_t data) noexcept -> void
{
    step();

    switch (address >> 12)
    {
        case 0x0 ... 0x7: m_cart->write(address, data);      return;
        case 0x8 ... 0x9: ppu.vram[address - 0x8000] = data; return;
        case 0xA ... 0xB: m_cart->write(address, data);      return;
        case 0xC ... 0xD: wram[address - 0xC000] = data;     return;

        case 0xF:
            switch (address & 0x0FFF)
            {
                case 0xF00:           joypad.byte = data;                               return;
                case 0xF01:           printf("%c", data);                               return;
                case 0xF02:                                                             return;
                case 0xF04:           timer.DIV = 0;                                    return;
                case 0xF05:           timer.TIMA = data;                                return;
                case 0xF06:           timer.TMA = data;                                 return;
                case 0xF07:           timer.TAC.byte = data;                            return;
                case 0xF0F:           interrupt_flag.byte = data;                       return;
                case 0xF10:           apu.CH1.NR10.byte = apu.set_register_check(data); return;
                case 0xF11:           apu.CH1.NR11.byte = apu.set_register_check(data); return;
                case 0xF12:           apu.CH1.NR12.byte = apu.set_register_check(data); return;
                case 0xF13:           apu.CH1.NR13      = apu.set_register_check(data); return;
                case 0xF14:           apu.set_NR14(data);                               return;
                case 0xF16:           apu.CH2.NR21.byte = apu.set_register_check(data); return;
                case 0xF17:           apu.CH2.NR22.byte = apu.set_register_check(data); return;
                case 0xF18:           apu.CH2.NR23      = apu.set_register_check(data); return;
                case 0xF19:           apu.set_NR24(data);                               return;
                case 0xF1A:           apu.CH3.NR30.byte = apu.set_register_check(data); return;
                case 0xF1B:           apu.CH3.NR31      = apu.set_register_check(data); return;
                case 0xF1C:           apu.CH3.NR32.byte = apu.set_register_check(data); return;
                case 0xF1D:           apu.CH3.NR33      = apu.set_register_check(data); return;
                case 0xF1E:           apu.set_NR34(data);                               return;
                case 0xF20:           apu.CH4.NR41.byte = apu.set_register_check(data); return;
                case 0xF21:           apu.CH4.NR42.byte = apu.set_register_check(data); return;
                case 0xF22:           apu.CH4.NR43.byte = apu.set_register_check(data); return;
                case 0xF23:           apu.set_NR44(data);                               return;
                case 0xF24:           apu.NR50.byte = apu.set_register_check(data);     return;
                case 0xF25:           apu.NR51.byte = apu.set_register_check(data);     return;
                case 0xF26:           apu.set_NR52(data);                               return;
                case 0xF30 ... 0xF3F: apu.set_wave_ram(address, data);                  return;
                case 0xF40:           ppu.set_LCDC(data);                               return;
                case 0xF41:           ppu.STAT.byte = data;                             return;
                case 0xF42:           ppu.SCY = data;                                   return;
                case 0xF43:           ppu.SCX = data;                                   return;

                // $FF46 - DMA - DMA Transfer and Start Address(W)
                case 0xF46:
                {
                    const auto addr{ data * 0x100 };

                    for (auto index{ 0 }; index < 160; ++index)
                    {
                        ppu.oam[index] = read(addr + index);
                    }
                    return;
                }

                case 0xF47:           ppu.BGP.byte = data;           return;
                case 0xF48:           ppu.OBP0.byte = data;          return;
                case 0xF49:           ppu.OBP1.byte = data;          return;
                case 0xF4A:           ppu.WY = data;                 return;
                case 0xF4B:           ppu.WX = data;                 return;
                case 0xF50:           boot_rom_disabled = true;      return;
                case 0xF80 ... 0xFFE: hram[address - 0xFF80] = data; return;
                case 0xFFF:           interrupt_enable.byte = data;  return;
                default:              __debugbreak();                return;
            }

        default: __debugbreak(); return;
    }
}
