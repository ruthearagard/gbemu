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

#include "bus.h"
#include "ppu.h"

using namespace GameBoy;

/// @brief Initializes the picture processing unit (PPU).
/// @param bus The system bus instance.
PPU::PPU(SystemBus& bus) noexcept : m_bus(bus)
{
    reset();
}

/// @brief Returns the value of the LCDC register.
/// @return The LCDC register.
auto PPU::get_LCDC() noexcept -> uint8_t
{
    return LCDC.byte;
}

/// @brief Updates LCDC and changes the state of the scanline renderer.
/// @param data The new LCDC value.
auto PPU::set_LCDC(const uint8_t data) noexcept -> void
{
    LCDC.byte = data;

    render_state.bg_tile_map     = LCDC.bg_tile_map ? 0x9C00 : 0x9800;
    render_state.window_tile_map = LCDC.window_tile_map ? 0x9C00 : 0x9800;

    render_state.sprite_size = LCDC.sprite_size ? 16 : 8;

    if (LCDC.bg_win_tile_data)
    {
        render_state.bg_win_tile_data = 0x8000;
        render_state.signed_tile_id = false;
    }
    else
    {
        render_state.bg_win_tile_data = 0x8800;
        render_state.signed_tile_id = true;
    }
}

/// @brief Returns a byte from VRAM using an absolute memory address.
/// @param index The absolute memory address.
/// @return The byte from VRAM.
auto PPU::vram_access(const unsigned int index) noexcept -> uint8_t
{
    return vram[index - 0x8000];
}

/// @brief Returns a byte from OAM using an absolute memory address.
/// @param index The absolute memory address.
/// @return The byte from OAM.
auto PPU::oam_access(const unsigned int index) noexcept -> uint8_t
{
    return oam[index - 0xFE00];
}

/// @brief Renders the current scanline.
auto PPU::draw_scanline() noexcept -> void
{
    unsigned int offset_x;
    unsigned int offset_y;
    uint16_t tile_map;
    uint16_t tile_data = render_state.bg_win_tile_data;

    auto will_draw{ false };

    if (LCDC.bg_enabled)
    {
        offset_x = (SCX + screen_x) & 0xFF;
        offset_y = (SCY + LY) & 0xFF;
        tile_map = render_state.bg_tile_map;

        will_draw = true;
    }

    if (LCDC.window_enabled)
    {
        const unsigned int m_WX = WX - 7;

        if ((WY <= LY) && (screen_x >= m_WX))
        {
            offset_x = screen_x - m_WX;
            offset_y = LY - WY;
            tile_map = render_state.window_tile_map;

            will_draw = true;
        }
    }

    if (will_draw)
    {
        const unsigned int row{ (offset_y / 8) * 32 };
        const unsigned int col{ offset_x / 8 };

        const unsigned int index{ tile_map + row + col };
            
        if (!render_state.signed_tile_id)
        {
            const uint8_t tile_id{ vram_access(index) };
            tile_data += tile_id * 16;
        }
        else
        {
            const int8_t tile_id{ static_cast<int8_t>(vram_access(index)) };
            tile_data += (tile_id + 128) * 16;
        }

        const unsigned int line{ (offset_y % 8) * 2 };
            
        const uint8_t lo{ vram_access(tile_data + line)     };
        const uint8_t hi{ vram_access(tile_data + line + 1) };

        pixel(lo, hi, 7 - (offset_x & 7), BGP, false);
    }

    if (LCDC.sprites_enabled)
    {
        for (unsigned int index{ 0xFE00 }; index < 0xFE9F; index += 4)
        {
            const uint8_t y = oam_access(index + 0) - 16;
            const uint8_t x = oam_access(index + 1) - 8;

            if ((LY >= y) && (LY < (y + render_state.sprite_size)))
            {
                if ((screen_x >= x) && (screen_x < (x + 8)))
                {
                    const unsigned int x_pos = screen_x - x;
                    const unsigned int line = (LY - y) * 2;

                    const uint8_t tile  = oam_access(index + 2);
                    const uint8_t flags = oam_access(index + 3);

                    const unsigned int address = 0x8000 + (tile * 16) + line;

                    const uint8_t lo{ vram_access(address)     };
                    const uint8_t hi{ vram_access(address + 1) };

                    Palette p;

                    if (flags & (1 << 4))
                    {
                        p = OBP1;
                    }
                    else
                    {
                        p = OBP0;
                    }
                    pixel(hi, lo, 7 - (x_pos & 7), p, true);
                }
            }
        }
    }
}

/// @brief Puts a pixel on the screen data.
/// @param lo The low byte of the tile data.
/// @param hi The high byte of the tile data.
/// @param bit The pixel bit to use.
/// @param palette The palette to use for color translation.
/// @param sprite Ignore color 0 if `true`.
auto PPU::pixel(const uint8_t hi,
                const uint8_t lo,
                const unsigned int bit,
                const Palette palette,
                const bool sprite) noexcept -> void
{
    const unsigned int p0{ (hi & (1 << bit)) != 0 };
    const unsigned int p1{ (lo & (1 << bit)) != 0 };

    const unsigned int pixel{ (p0 << 1) | p1 };

    constexpr std::array<enum Colors, 4> colors =
    {
        Colors::White,
        Colors::LightGray,
        Colors::DarkGray,
        Colors::Black
    };

    const unsigned int index{ (LY * SCREEN_X) + screen_x };

    switch (pixel)
    {
        case 0:
            if (!sprite)
            {
                screen_data[index] = colors[palette.c0];
            }
            return;

        case 1:
            screen_data[index] = colors[palette.c1];
            return;

        case 2:
            screen_data[index] = colors[palette.c2];
            return;

        case 3:
            screen_data[index] = colors[palette.c3];
            return;
    }
}

/// @brief Resets the PPU to the startup state.
auto PPU::reset() noexcept -> void
{
    set_LCDC(0x91);

    SCX = 0x00;
    SCY = 0x00;
    BGP.byte = 0xFC;

    STAT.byte = 0x00;

    WY = 0x00;
    WX = 0x00;

    ly_counter = 0;
    screen_x = 0;

    vram        = { };
    screen_data = { };
}

/// @brief Advances the PPU by 1 m-cycle.
auto PPU::step() noexcept -> void
{
    // We do nothing if the PPU isn't enabled.
    if (!LCDC.enabled)
    {
        LY = 0x00;

        STAT.mode = Mode::VBlankOrDisabled;

        screen_data = { };
        ly_counter = 0;

        return;
    }

    ly_counter += 4;

    switch (STAT.mode)
    {
        case Mode::HBlank:
            if (ly_counter == 204)
            {
                LY++;

                if (LY == 144)
                {
                    m_bus.irq(Interrupt::VBlankInterrupt);
                    STAT.mode = Mode::VBlankOrDisabled;
                }
                else
                {
                    STAT.mode = Mode::OAMSearch;
                }
                ly_counter = 0;
            }
            break;

        case Mode::VBlankOrDisabled:
            if (ly_counter == 456)
            {
                LY++;

                if (LY == 154)
                {
                    STAT.mode = Mode::OAMSearch;
                    LY   = 0;
                }
                ly_counter = 0;
            }
            break;

        case Mode::OAMSearch:
            if (ly_counter == 80)
            {
                STAT.mode = Mode::Drawing;
                ly_counter = 0;
            }
            break;

        case Mode::Drawing:
            if (ly_counter == 172)
            {
                for (; screen_x < SCREEN_X; ++screen_x)
                {
                    draw_scanline();
                }

                screen_x = 0;
                ly_counter = 0;
                STAT.mode = Mode::HBlank;
            }
            break;
    }
}
