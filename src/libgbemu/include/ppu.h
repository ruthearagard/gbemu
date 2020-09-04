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
// `#ifndef` to guard the contents of header files against multiple
// inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

#include <array>
#include <cstdint>

namespace GameBoy
{
    class SystemBus;

    /// @brief The maximum length of a line.
    constexpr auto SCREEN_X{ 160 };

    /// @brief The maximum height of the screen.
    constexpr auto SCREEN_Y{ 144 };

    /// @brief Alias for the screen data.
    using ScreenData = std::array<uint32_t, SCREEN_X * SCREEN_Y>;

    /// @brief Defines the picture processing unit (PPU).
    class PPU
    {
    public:
        /// @brief Initializes the picture processing unit (PPU).
        /// @param bus The system bus instance.
        explicit PPU(SystemBus& bus) noexcept;

        /// @brief Updates LCDC and changes the state of the scanline renderer.
        /// @param data The new LCDC value.
        auto set_LCDC(const uint8_t data) noexcept -> void;

        /// @brief Resets the PPU to the startup state.
        auto reset() noexcept -> void;

        /// @brief Advances the PPU by 1 m-cycle.
        auto step() noexcept -> void;

        union
        {
            struct
            {
                unsigned int bg_enabled : 1;
                unsigned int sprites_enabled : 1;
                unsigned int sprite_size : 1;
                unsigned int bg_tile_map : 1;
                unsigned int bg_win_tile_data : 1;
                unsigned int window_enabled : 1;
                unsigned int window_tile_map : 1;
                unsigned int enabled : 1;
            };
            uint8_t byte;
        } LCDC;

        union
        {
            struct
            {
                unsigned int mode : 2;
                unsigned int lyc_eq_ly : 1;
                unsigned int hblank_interrupt_enabled : 1;
                unsigned int vblank_interrupt_enabled : 1;
                unsigned int oam_interrupt_enabled : 1;
                unsigned int lyc_eq_ly_interrupt_enabled : 1;
            };
            uint8_t byte;
        } STAT;

        /// @brief Scroll Y
        //
        // Specifies the Y position in the 256x256 pixels BG map (32x32 tiles)
        // which is to be displayed on the LCD.
        uint8_t SCY;

        /// @brief Scroll X
        //
        // Specifies the X position in the 256x256 pixels BG map (32x32 tiles)
        // which is to be displayed on the LCD.
        uint8_t SCX;

        /// @brief Current vertical line (R)
        ///
        /// Indicates the vertical line to which the present data is transferred
        /// to the LCD. It can take on any value between 0 and 153. Values
        /// between 144 and 153 indicate the V-Blank period.
        uint8_t LY;

        /// @brief BG Palette Data
        ///
        /// This register assigns gray shades to the color numbers of the BG and
        /// Window tiles.
        //
        /// The four possible gray shades are:
        ///
        /// 0 - White
        /// 1 - Light gray
        /// 2 - Dark gray
        /// 3 - Black
        union Palette
        {
            struct
            {
                unsigned int c0 : 2;
                unsigned int c1 : 2;
                unsigned int c2 : 2;
                unsigned int c3 : 2;
            };
            uint8_t byte;
        } BGP, OBP0, OBP1;

        // $FF4A - WY - Window Y Position (R/W)
        uint8_t WY;

        // $FF4B - WX - Window X Position minus 7 (R/W)
        uint8_t WX;

        // [$8000 - $9FFF] - 8KB Video RAM (VRAM)
        std::array<uint8_t, 8192> vram;

        // [$FE00 - $FE9F]: Sprite Attribute Table (OAM)
        std::array<uint8_t, 160> oam;

        /// @brief Screen data to be displayed to the host machine (RGBA32)
        ScreenData screen_data;

        unsigned int ly_counter;

    private:
        /// @brief Returns a byte from VRAM using an absolute memory address.
        /// @param index The absolute memory address.
        /// @return The byte from VRAM.
        auto vram_access(const unsigned int index) noexcept -> uint8_t;

        /// @brief Returns a byte from OAM using an absolute memory address.
        /// @param index The absolute memory address.
        /// @return The byte from OAM.
        auto oam_access(const unsigned int index) noexcept -> uint8_t;

        /// @brief Renders the current scanline.
        auto draw_scanline() noexcept -> void;

        auto pixel(const uint8_t lo,
                   const uint8_t hi,
                   const unsigned int bit,
                   const Palette palette,
                   const bool sprite) noexcept -> void;

        /// @brief Current X position of the scanline being drawn
        unsigned int screen_x;

        // @brief RGBA32 color values used for the screen data
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

        /// @brief Render state set up by the call to `set_LCDC()`.
        struct
        {
            /// @brief Beginning address of background tile map
            /// (must be $9800 or $9C00).
            uint16_t bg_tile_map;

            /// @brief Beginning address of window tile map
            /// (must be $9800 or $9C00).
            uint16_t window_tile_map;

            /// @brief Beginning address of tile data for both the window and
            /// background (must be $8000 or $8800). In the latter case, the
            /// tile IDs are signed.
            uint16_t bg_win_tile_data;

            /// @brief Size of sprites (must be 8 or 16).
            unsigned int sprite_size;

            /// @brief Will the tile IDs in background/window tile data area be
            /// signed?
            bool signed_tile_id;
        } render_state;

        /// @brief System bus instance
        SystemBus& m_bus;
    };
}