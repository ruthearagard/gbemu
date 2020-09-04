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

// Required for the `GameBoy::ROMOnlyCartridge` class.
#include "rom_only.h"

using namespace GameBoy;

ROMOnlyCartridge::ROMOnlyCartridge(const std::vector<uint8_t>& data) noexcept :
Cartridge(data)
{ }

/// @brief Returns a byte from the cartridge.
/// @param address The memory address to read from.
/// @return The byte from the cartridge.
auto ROMOnlyCartridge::read(const uint16_t address) noexcept -> uint8_t
{
    return m_data[address];
}

/// @brief Updates the memory bank controller configuration.
/// 
/// This should never happen here as a memory bank controller is not present.
/// Such writes will be logged.
/// 
/// @param address The configuration area to update.
/// @param value The value to update the area with.
auto ROMOnlyCartridge::write(const uint16_t address,
                             const uint8_t value) noexcept -> void
{
    __debugbreak();
}