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

// Required for the `GameBoy::Timer` class.
#include "timer.h"

using namespace GameBoy;

Timer::Timer(SystemBus& m_bus) noexcept : bus(m_bus)
{
    reset();
}

// Resets the timers to the startup state.
auto Timer::reset() noexcept -> void
{
    TIMA = 0x00;
    TMA  = 0x00;

    TAC.byte = 0x00;

    div_counter = 0;
    tima_counter = 0;
}

// Advances the timers by 1 m-cycle.
auto Timer::step() noexcept -> void
{
    div_counter += 4;

    if (div_counter == 256)
    {
        DIV++;
        div_counter = 0;
    }

    if (TAC.active)
    {
        tima_counter += 4;

        unsigned int threshold{ 0 };

        switch (TAC.input_clock)
        {
            case 0: threshold = 1024; break;
            case 1: threshold = 16;   break;
            case 2: threshold = 64;   break;
            case 3: threshold = 256;  break;
        }

        while (tima_counter >= threshold)
        {
            if (TIMA == 0xFF)
            {
                bus.signal_interrupt(Interrupt::TimerInterrupt);
                TIMA = TMA;
            }
            TIMA++;
            tima_counter -= threshold;
        }
    }
}