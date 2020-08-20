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
    class APU
    {
    public:
        // $FF24 - NR50 - Channel control / ON-OFF / Volume (R/W)
        //
        // The volume bits specify the "Master Volume" for Left/Right sound
        // output.
        //
        // Bit 7:   Output Vin to SO2 terminal (1=Enable)
        // Bit 6-4: SO2 output level (volume)  (0-7)
        // Bit 3:   Output Vin to SO1 terminal (1=Enable)
        // Bit 2-0: SO1 output level (volume)  (0-7)
        //
        // The Vin signal is received from the game cartridge bus, allowing
        // external hardware in the cartridge to supply a fifth sound channel,
        // additionally to the Game Boy's internal four channels.
        //
        // As far as I know, this feature isn't used by any existing games.
        uint8_t NR50;

        // $FF25 - NR51 - Selection of Sound output terminal (R/W)
        //
        // Bit 7: Output sound 4 to SO2 terminal
        // Bit 6: Output sound 3 to SO2 terminal
        // Bit 5: Output sound 2 to SO2 terminal
        // Bit 4: Output sound 1 to SO2 terminal
        // Bit 3: Output sound 4 to SO1 terminal
        // Bit 2: Output sound 3 to SO1 terminal
        // Bit 1: Output sound 2 to SO1 terminal
        // Bit 0: Output sound 1 to SO1 terminal
        uint8_t NR51;

        // $FF26 - NR52 - Sound on/off
        //
        // If your GB programs don't use sound, then write $00 to this register
        // to save 16% or more on GB power consumption. Disabling the sound
        // controller by clearing bit 7 destroys the contents of all sound
        // registers.
        //
        // Also, it is not possible to access any sound registers
        // (except $FF26) while the sound controller is disabled.
        //
        // Bit 7: All sound on/off (0: stop all sound circuits) (R/W)
        // Bit 3: Sound 4 ON flag (R)
        // Bit 2: Sound 3 ON flag (R)
        // Bit 1: Sound 2 ON flag (R)
        // Bit 0: Sound 1 ON flag (R)
        //
        // Bits 0-3 of this register are read only status bits, writing to
        // these bits does NOT enable/disable sound. The flags get set when
        // sound output is restarted by setting the Initial flag
        // (bit 7 in NR14 - NR44), the flag remains set until the sound length
        // has expired, if enabled. A volume envelopes which has decreased to
        // zero volume will NOT cause the sound flag to go off.
        uint8_t NR52;
    };
}