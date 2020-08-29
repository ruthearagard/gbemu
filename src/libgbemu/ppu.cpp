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
    LCDC = data;

    bg_tile_map = (LCDC & (1 << 3)) ? 0x9C00 : 0x9800;

    if (LCDC & (1 << 4))
    {
        bg_win_tile_data = 0x8000;
    }
    else
    {
        bg_win_tile_data = 0x8800;
        signed_tile_id = true;
    }
}

auto PPU::draw_scanline(const unsigned int x) noexcept -> void
{
    // Is the background enabled?
    if (LCDC & 1)
    {
        const unsigned int offset_x = SCX + x;
        const unsigned int offset_y = SCY + LY;

        const unsigned int row = (offset_y / 8) * 32;
        const unsigned int col = offset_x / 8;

        const unsigned int index{ bg_tile_map + row + col };

        uint16_t data = bg_win_tile_data;
        int16_t tile_id;

        if (!signed_tile_id)
        {
            tile_id = vram_access(index);
            data += tile_id * 16;
        }
        else
        {
            tile_id = static_cast<int16_t>(vram_access(index));
            data += (tile_id + 128) * 16;
        }

        const unsigned int line = (offset_y % 8) * 2;

        const uint8_t lo = vram_access(data + line);
        const uint8_t hi = vram_access(data + line + 1);

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
            screen_data[index] = colors[BGP & 0x03];
            return;

        case 1:
            screen_data[index] = colors[(BGP >> 2) & 0x03];
            return;

        case 2:
            screen_data[index] = colors[(BGP >> 4) & 0x03];
            return;

        case 3:
            screen_data[index] = colors[(BGP >> 6) & 0x03];
            return;
    }
}

// Resets the PPU to the startup state.
auto PPU::reset() noexcept -> void
{
    set_LCDC(0x91);

    SCX  = 0x00;
    SCY  = 0x00;
    BGP  = 0xFC;

    STAT = 0x00;

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
    if (!(LCDC & 0x80))
    {
        LY = 0x00;
        STAT = (STAT & ~0x03) | (Mode::VBlankOrDisabled & 0x03);

        screen_data = { };
        ly_counter = 0;

        return;
    }

    ly_counter += 4;

    switch (STAT & 0x03)
    {
        case Mode::HBlank:
            if (ly_counter == 204)
            {
                LY++;

                if (LY == 144)
                {
                    STAT = (STAT & ~0x03) | (Mode::VBlankOrDisabled & 0x03);
                }
                else
                {
                    STAT = (STAT & ~0x03) | (Mode::OAMSearch & 0x03);
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
                    STAT = (STAT & ~0x03) | (Mode::OAMSearch & 0x03);
                    LY   = 0;
                }
                ly_counter = 0;
            }
            break;

        case Mode::OAMSearch:
            if (ly_counter == 80)
            {
                STAT = (STAT & ~0x03) | (Mode::Drawing & 0x03);
                ly_counter = 0;
            }
            break;

        case Mode::Drawing:
            if (ly_counter == 172)
            {
                for (; screen_x < ScreenX; ++screen_x)
                {
                    draw_scanline(screen_x);
                }

                screen_x = 0;
                ly_counter = 0;
                STAT = (STAT & ~0x03) | (Mode::HBlank & 0x03);
            }
            break;
    }
}