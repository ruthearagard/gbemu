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

// Required for the `std::runtime_error()` exception.
#include <stdexcept>

// Required for the `GameBoy::Cartridge` class.
#include "cart.h"

using namespace GameBoy;

Cartridge::Cartridge(const std::vector<uint8_t>& data) noexcept : m_data(data)
{
    // First, we check to see if the header checksum is valid. We use the
    // `at()` method to access the cartridge data since we're not sure if the
    // indicies referenced below actually exist, and referencing them using
    // `operator[]` could lead to undefined behavior.
    unsigned int x{ 0 };

    for (unsigned int i{ 0x0134 }; i <= 0x014C; ++i)
    {
        try
        {
            x = x - m_data.at(i) - 1;
        }
        catch (std::out_of_range&)
        {
            throw std::runtime_error("Header checksum verification failed.");
        }
    }

    uint8_t checksum;

    try
    {
        checksum = m_data.at(0x014D);
    }
    catch (std::out_of_range&)
    {
        throw std::runtime_error("Header checksum verification failed.");
    }

    if ((x & 0xFF) != checksum)
    {
        throw std::runtime_error("Header checksum verification failed.");
    }
}

// Returns a byte from the cartridge data referenced by memory address
// `address`.
auto Cartridge::read(const uint16_t address) const noexcept -> uint8_t
{
    return m_data[address];
}
