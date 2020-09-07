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

#include "apu.h"

using namespace GameBoy;

/// @brief
/// @param data
/// @return 0 if sound is disabled, or `data` otherwise.
auto APU::set_register_check(const uint8_t data) noexcept -> uint8_t
{
    if (NR52.enabled)
    {
        return data;
    }
    return 0x00;
}

/// @brief Sets wave RAM data.
/// @param address The index of the wave RAM data.
/// @param data The data to place in the wave RAM index.
auto APU::set_wave_ram(const uint16_t address,
                       const uint8_t data) noexcept -> void
{
    if (NR52.enabled)
    {
        CH3.ram[address - 0xFF30] = data;
    }
}

/// @brief Sets the $FF14 - NR14 - Channel 1 Frequency hi (R/W) register. Does
/// nothing if sound is disabled.
/// @param data The data to set the register to.
auto APU::set_NR14(const uint8_t data) noexcept -> void
{
    if (data & 0x80)
    {
        NR52.ch1_on = true;
    }
}

/// @brief Sets the $FF19 - NR24 - Channel 2 Frequency hi data (R/W) register.
/// Does nothing if sound is disabled.
/// @param data The data to set the register to.
auto APU::set_NR24(const uint8_t data) noexcept -> void
{
    if (data & 0x80)
    {
        NR52.ch2_on = true;
    }
}

/// @brief Sets the $FF1E - NR34 - Channel 3 Frequency's higher data (R/W)
/// register. Does nothing if sound is disabled.
/// @param data The data to set the register to.
auto APU::set_NR34(const uint8_t data) noexcept -> void
{
    if (data & 0x80)
    {
        NR52.ch3_on = true;
    }
}

/// @brief Sets the $FF23 - NR44 - Channel 4 Counter/consecutive; Inital (R/W)
/// register. Does nothing if sound is disabled.
/// @param data The data to set the register to.
auto APU::set_NR44(const uint8_t data) noexcept -> void
{
    if (data & 0x80)
    {
        NR52.ch4_on = true;
    }
}

/// @brief Sets the $FF26 - NR52 - Sound on/off register.
/// @param data The data to set the register to.
auto APU::set_NR52(const uint8_t data) noexcept -> void
{
    const auto msb{ (data & 0x80) != 0 };

    if (!msb)
    {
        reset();
    }
    NR52.enabled = msb;
}

/// @brief Resets the APU to the startup state.
auto APU::reset() noexcept -> void
{
    CH1 = { };
    CH2 = { };
    CH3 = { };
    CH4 = { };

    NR50 = { };
    NR51 = { };
    NR52 = { };

    frame_sequencer = 0;
    frame_sequencer_step = 0;
}

/// @brief Steps the APU by 1 m-cycle.
auto APU::step() noexcept -> void
{
    if (NR52.enabled)
    {
        frame_sequencer += 4;

        if (frame_sequencer == 8192)
        {
            switch (frame_sequencer_step)
            {
                // Decrement length counters
                case 0:
                case 4:
                    break;

                // Decrement length counters and calculate sweep frequency
                case 2:
                case 6:
                    break;

                // Calculate new volume
                case 7:
                    break;
            }
        }
        frame_sequencer = 0;
        frame_sequencer_step++;

        if (frame_sequencer_step > 7)
        {
            frame_sequencer_step = 0;
        }
    }
}
