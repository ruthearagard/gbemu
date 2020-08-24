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

// Required for fixed-width integer types (e.g. `uint8_t`).
#include <cstdint>

namespace GameBoy
{
    // Forward declaration
    class SystemBus;

    class Timer
    {
    public:
        explicit Timer(SystemBus& m_bus) noexcept;

        // Resets the timers to the startup state.
        auto reset() noexcept -> void;

        // Advances the timers by 1 m-cycle.
        auto step() noexcept -> void;

        // $FF04 - DIV - Divider Register (R/W)
        //
        // This register is incremented at rate of 16384Hz. Writing any value
        // to this register resets it to $00.
        uint8_t DIV;

        // $FF05 - TIMA - Timer counter (R/W)
        //
        // This timer is incremented by a clock frequency specified by the TAC
        // register ($FF07). When the value overflows (gets bigger than $FF)
        // then it will be reset to the value specified in TMA ($FF06), and an
        // interrupt will be requested, as described below.
        uint8_t TIMA;

        // $FF06 - TMA - Timer Modulo (R/W)
        //
        // When the TIMA overflows, this data will be loaded.
        uint8_t TMA;

        // $FF07 - TAC - Timer Control (R/W)
        //
        // Bit 2 - Timer Stop (0=Stop, 1=Start)
        //
        // Bits 1-0: Input Clock Select
        //
        // 00 : 4096 Hz(~4194 Hz SGB)
        // 01 : 262144 Hz(~268400 Hz SGB)
        // 10 : 65536 Hz(~67110 Hz SGB)
        // 11 : 16384 Hz(~16780 Hz SGB)
        uint8_t TAC;

        unsigned int div_counter;
        unsigned int tima_counter;

    private:
        // System bus instance
        SystemBus& bus;
    };
}