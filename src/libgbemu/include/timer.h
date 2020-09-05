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

#include <cstdint>

namespace GameBoy
{
    class SystemBus;

    /// @brief Defines the timer device.
    class Timer final
    {
    public:
        explicit Timer(SystemBus& m_bus) noexcept;

        /// @brief Resets the timer to the startup state.
        auto reset() noexcept -> void;

        /// @brief Advances the timer by 1 m-cycle.
        auto step() noexcept -> void;

        /// @brief Divider
        ///
        /// This register is incremented at rate of 16384Hz. Writing any value
        /// to this register resets it to $00.
        uint8_t DIV;

        /// @brief Timer value
        ///
        /// This value is incremented by a clock frequency specified by the TAC
        /// register ($FF07). When the value overflows (i.e. becomes >$FF),
        /// then it will be reset to the value specified in TMA ($FF06), and a
        /// timer interrupt will be requested.
        uint8_t TIMA;

        /// @brief Timer Modulo (R/W)
        ///
        /// When the TIMA overflows, this data will be loaded.
        uint8_t TMA;

        /// @brief TAC - Timer Control (R/W)
        union
        {
            struct
            {
                /// @brief Specifies the frequency in which TIMA is to be
                /// incremented.
                /// 
                /// 0: 4096 Hz
                /// 1: 262144 Hz
                /// 2: 65536 Hz
                /// 3: 16384 Hz
                unsigned int input_clock : 2;

                /// @brief Bit 2 - Timer Stop (0=Stop, 1=Start)
                unsigned int active : 1;

                unsigned int : 5;
            };
            uint8_t byte;
        } TAC;

        /// @brief Current DIV cycle counter
        uint16_t div_counter;

        /// @brief Current TIMA cycle counter
        uint16_t tima_counter;

    private:
        /// @brief System bus instance
        SystemBus& bus;
    };
}