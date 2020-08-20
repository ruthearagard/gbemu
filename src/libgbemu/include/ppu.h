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

// Required for `std::array<>`.
#include <array>

// Required for fixed-width integer types (e.g. `uint8_t`).
#include <cstdint>

namespace GameBoy
{
    class PPU
    {
    public:
        // $FF40 - LCDC - LCD Control (R/W)
        //
        // Bit 7 - LCD Display Enable (0=Off, 1=On)
        // Bit 6 - Window Tile Map Display Select (0=$9800-$9BFF, 1=$9C00-$9FFF)
        // Bit 5 - Window Display Enable (0=Off, 1=On)
        // Bit 4 - BG & Window Tile Data Select (0=$8800-$97FF, 1=$8000-$8FFF)
        // Bit 3 - BG Tile Map Display Select (0 =$9800-$9BFF, 1=$9C00-$9FFF)
        // Bit 2 - OBJ(Sprite) Size (0=8x8, 1=8x16)
        // Bit 1 - OBJ(Sprite) Display Enable (0=Off, 1=On)
        // Bit 0 - BG Display (0=Off, 1=On)
        uint8_t LCDC;

        // $FF43 - SCX - Scroll X (R/W)
        //
        // Specifies the position in the 256x256 pixels BG map (32x32 tiles)
        // which is to be displayed at the upper/left LCD display position.
        //
        // Values in range from 0-255 may be used for X/Y each, the video
        // controller automatically wraps back to the upper(left) position in
        // BG map when drawing exceeds the lower(right) border of the BG map
        // area.
        uint8_t SCX;

        // $FF44 - LY - LCDC Y-Coordinate (R)
        //
        // The LY indicates the vertical line to which the present data is
        // transferred to the LCD Driver. The LY can take on any value between
        // 0 through 153. The values between 144 and 153 indicate the V-Blank
        // period. Writing will reset the counter.
        uint8_t LY = 0x90;

        // $FF47 - BGP - BG Palette Data (R/W)
        //
        // This register assigns gray shades to the color numbers of the BG and
        // Window tiles.
        //
        // Bit 7-6: Shade for Color Number 3
        // Bit 5-4: Shade for Color Number 2
        // Bit 3-2: Shade for Color Number 1
        // Bit 1-0: Shade for Color Number 0
        //
        // The four possible gray shades are:
        //
        // 0 - White
        // 1 - Light gray
        // 2 - Dark gray
        // 3 - Black
        uint8_t BGP;

        // [$8000 - $9FFF] - 8KB Video RAM (VRAM)
        std::array<uint8_t, 8192> vram;
    };
}