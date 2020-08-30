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

// Required for the `GameBoy::PPU` class.
#include "ppu.h"

using namespace GameBoy;

PPU::PPU(SystemBus& bus) noexcept : m_bus(bus)
{
    reset();
}

// Updates LCDC and changes the internal state of the scanline renderer.
auto PPU::set_LCDC(const uint8_t data) noexcept -> void
{
    LCDC.byte = data;

    bg_tile_map = LCDC.bg_tile_map ? 0x9C00 : 0x9800;

    if (LCDC.bg_win_tile_data)
    {
        bg_win_tile_data = 0x8000;
        signed_tile_id = false;
    }
    else
    {
        bg_win_tile_data = 0x8800;
        signed_tile_id = true;
    }
}

auto PPU::draw_scanline() noexcept -> void
{
    // Is the background enabled?
    if (LCDC.bg_enabled)
    {
        const unsigned int offset_x{ SCX + screen_x };
        const unsigned int offset_y = SCY + LY;

        const unsigned int row{ (offset_y / 8) * 32 };
        const unsigned int col{ offset_x / 8 };

        const unsigned int index{ bg_tile_map + row + col };

        uint16_t data{ bg_win_tile_data };
        uint8_t tile_id{ vram_access(index) };

        if (!signed_tile_id)
        {
            data += tile_id * 16;
        }
        else
        {
            data += (tile_id + 128) * 16;
        }

        const unsigned int line{ (offset_y % 8) * 2 };

        const uint8_t lo{ vram_access(data + line)     };
        const uint8_t hi{ vram_access(data + line + 1) };

        pixel(lo, hi, ((offset_x % 8) - 7) * -1);
    }
}

auto PPU::pixel(const uint8_t lo,
                const uint8_t hi,
                const unsigned int bit) noexcept -> void
{
    const unsigned int p0{ (lo & (1 << bit)) != 0 };
    const unsigned int p1{ (hi & (1 << bit)) != 0 };

    const unsigned int pixel{ (p1 << 1) | p0 };

    static const std::array<enum Colors, 4> colors =
    {
        Colors::White,
        Colors::LightGray,
        Colors::DarkGray,
        Colors::Black
    };

    const unsigned int index{ (LY * ScreenX) + screen_x };

    switch (pixel)
    {
        case 0:
            screen_data[index] = colors[BGP.c0];
            return;

        case 1:
            screen_data[index] = colors[BGP.c1];
            return;

        case 2:
            screen_data[index] = colors[BGP.c2];
            return;

        case 3:
            screen_data[index] = colors[BGP.c3];
            return;
    }
}

// Resets the PPU to the startup state.
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

    vram        = { };
    screen_data = { };
}

// Advances the PPU by 1 m-cycle.
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
                    m_bus.signal_interrupt(Interrupt::VBlankInterrupt);
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
                for (; screen_x < ScreenX; ++screen_x)
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