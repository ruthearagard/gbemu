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
    // Forward declaration
    class SystemBus;

    class PPU
    {
    public:
        explicit PPU(SystemBus& bus) noexcept;

        // Resets the PPU to the startup state.
        auto reset() noexcept -> void;

        // Advances the PPU by 1 m-cycle.
        auto step() noexcept -> void;

        // Updates LCDC and changes the internal state of the scanline
        // renderer.
        auto set_LCDC(const uint8_t data) noexcept -> void;

        // $FF40 - LCDC - LCD Control (R/W)
        //
        // Bit 7 - LCD Display Enable (0=Off, 1=On)
        // Bit 6 - Window Tile Map Display Select (0=$9800-$9BFF, 1=$9C00-$9FFF)
        // Bit 5 - Window Display Enable (0=Off, 1=On)
        // Bit 4 - BG & Window Tile Data Select (0=$8800-$97FF, 1=$8000-$8FFF)
        // Bit 3 - BG Tile Map Display Select (0=$9800-$9BFF, 1=$9C00-$9FFF)
        // Bit 2 - OBJ(Sprite) Size (0=8x8, 1=8x16)
        // Bit 1 - OBJ(Sprite) Display Enable (0=Off, 1=On)
        // Bit 0 - BG Display (0=Off, 1=On)
        uint8_t LCDC;

        // $FF41 - STAT - LCDC Status (R/W)
        //
        // Bit 6 - LYC=LY Coincidence Interrupt (1=Enable) (R/W)
        // Bit 5 - Mode 2 OAM Interrupt (1=Enable) (R/W)
        // Bit 4 - Mode 1 V-Blank Interrupt (1=Enable) (R/W)
        // Bit 3 - Mode 0 H-Blank Interrupt (1=Enable) (R/W)
        // Bit 2 - Coincidence Flag (0:LYC!=LY, 1: LYC=LY) (R)
        // Bit 1-0: Mode Flag(Mode 0-3) (R)
        //
        // 0: During H-Blank
        // 1: During V-Blank
        // 2: During Searching OAM RAM
        // 3: During Transfering Data to LCD Driver
        uint8_t STAT;

        // $FF42 - SCY - Scroll Y (R/W)
        //
        // Specifies the position in the 256x256 pixels BG map (32x32 tiles)
        // which is to be displayed at the upper/left LCD display position.
        //
        // Values in range from 0-255 may be used for X/Y each, the video
        // controller automatically wraps back to the upper(left) position in
        // BG map when drawing exceeds the lower(right) border of the BG map
        // area.
        uint8_t SCY;

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
        uint8_t LY;

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

        // $FF4A - WY - Window Y Position (R/W)
        uint8_t WY;

        // $FF4B - WX - Window X Position minus 7 (R/W)
        uint8_t WX;

        // [$8000 - $9FFF] - 8KB Video RAM (VRAM)
        std::array<uint8_t, 8192> vram;

        // Screen data (BGRA32)
        std::array<uint32_t, 160 * 144> screen_data;

        unsigned int ly_counter;

        static constexpr auto ScreenX{ 160 };
        static constexpr auto ScreenY{ 144 };

    private:
        auto draw_scanline(const unsigned int x) noexcept -> void;

        auto pixel(const uint8_t lo,
                   const uint8_t hi,
                   const unsigned int bit) noexcept -> void;

        // Current X position of the current scanline being drawn
        unsigned int screen_x;

        // BGRA32
        enum Colors : uint32_t
        {
            White     = 0x00FFFFFF,
            LightGray = 0x00D3D3D3,
            DarkGray  = 0x00A9A9A9,
            Black     = 0x00000000
        };

        enum Mode
        {
            HBlank,
            VBlankOrDisabled,
            OAMSearch,
            Drawing
        };

        inline auto vram_access(const unsigned int index) noexcept -> uint8_t
        {
            return vram[index - 0x8000];
        }

        // Beginning address of background tile map (must be $9800 or $9C00).
        uint16_t bg_tile_map;

        // Beginning address of tile data for both the window and background
        // (must be $8000 or $8800). In the latter case, the tile IDs are
        // signed.
        uint16_t bg_win_tile_data;

        // Will the tile IDs in background/window tile data area be signed?
        bool signed_tile_id;

        // System bus instance
        SystemBus& m_bus;
    };
}