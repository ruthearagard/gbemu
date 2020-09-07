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

/// @brief Updates channel 1 state.
auto APU::update_ch1_state(const uint8_t data) noexcept -> void
{
    if (data & 0x80)
    {
        // Writing a value with bit 7 set causes the following things to occur:

        // 1. Channel is enabled (see length counter).
        // 2. If length counter is zero, it is set to 63.
        if (CH1.length_duty.length == 0)
        {
            CH1.length_duty.length = 63;
        }

        // 3. Frequency timer is reloaded with period.
        // 4. Volume envelope timer is reloaded with period.
        // 4. Volume envelope timer is reloaded with period.
        // 5. Channel volume is reloaded from NR12.
        // 6. Square 1's sweep does several things (see frequency sweep).
    }
}

/// @brief Updates channel 2 state.
auto APU::update_ch2_state(const uint8_t data) noexcept -> void
{
    if (data & 0x80)
    {
        // Writing a value with bit 7 set causes the following things to occur:

        // 1. Channel is enabled(see length counter).
        // 2. If length counter is zero, it is set to 63.
        if (CH2.length_duty.length == 0)
        {
            CH2.length_duty.length = 63;
        }

        // 3. Frequency timer is reloaded with period.
        // 4. Volume envelope timer is reloaded with period.
        // 5. Channel volume is reloaded from NR22.
    }
}

/// @brief Updates channel 3 state.
auto APU::update_ch3_state(const uint8_t data) noexcept -> void
{

}

/// @brief Updates channel 4 state.
auto APU::update_ch4_state(const uint8_t data) noexcept -> void
{
    if (data & 0x80)
    {
        // Writing a value with bit 7 set causes the following things to occur:
        //
        // 1. Channel is enabled(see length counter).
        // 2. If length counter is zero, it is set to 63.
        if (CH4.length.length == 0)
        {
            CH4.length.length = 63;
        }

        // 3. Frequency timer is reloaded with period.
        // 4. Volume envelope timer is reloaded with period.
        // 5. Channel volume is reloaded from NR32.
        // 6. Noise channel's LFSR bits are all set to 1.
    }
}

/// @brief Resets the APU to the startup state.
auto APU::reset() noexcept -> void
{
    CH1 = { };
    CH2 = { };
    CH3 = { };
    CH4 = { };

    output_terminal = { };
    sound_control   = { };
    channel_control = { };

    frame_sequencer = 0;
    frame_sequencer_step = 0;
}

/// @brief Steps the APU by 1 m-cycle.
auto APU::step() noexcept -> void
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

        frame_sequencer = 0;
        frame_sequencer_step++;

        if (frame_sequencer_step > 7)
        {
            frame_sequencer_step = 0;
        }
    }
}
