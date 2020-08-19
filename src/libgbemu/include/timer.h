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
    class Timer
    {
    public:
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
    };
}