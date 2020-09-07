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

#include <stdexcept>
#include <fmt/printf.h>
#include "../cart/mbc1.h"
#include "../cart/mbc3.h"
#include "../cart/mbc1.h"
#include "../cart/rom_only.h"
#include "gb.h"

using namespace GameBoy;

/// @brief Initializes a Game Boy system.
System::System() noexcept : cpu(bus)
{
    reset();
}

/// @brief Resets the system to the startup state.
auto System::reset() noexcept -> void
{
    bus.reset();
    cpu.reset();
}

/// @brief Presses a button on the virtual joypad.
/// @param button The button to press.
auto System::press_button(const JoypadButton button) noexcept -> void
{
    bus.joypad_state &= ~button;
}

/// @brief Releases a button on the virtual joypad.
/// @param button The button to release.
auto System::release_button(const JoypadButton button) noexcept -> void
{
    bus.joypad_state |= button;
}

/// @brief Generates a cartridge.
/// 
/// This function will throw an `std::runtime_error` under the following
/// circumstances:
/// 
/// * The header checksum verification failed, or;
/// * The ROM requires a memory bank controller we don't support.
/// 
/// @param cart_data The data to use to generate the Cartridge instance.
/// @return The generated cartridge.
auto System::cart(const std::vector<uint8_t>& cart_data) ->
std::shared_ptr<Cartridge>
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
            x = x - cart_data.at(i) - 1;
        }
        catch (std::out_of_range&)
        {
            throw std::runtime_error("Header checksum verification failed.");
        }
    }

    uint8_t checksum;

    try
    {
        checksum = cart_data.at(0x014D);
    }
    catch (std::out_of_range&)
    {
        throw std::runtime_error("Header checksum verification failed.");
    }

    if ((x & 0xFF) != checksum)
    {
        throw std::runtime_error("Header checksum verification failed.");
    }

    std::shared_ptr<GameBoy::Cartridge> cart;

    // Okay, this is a valid cartridge. But do we support the memory bank
    // controller it needs?
    switch (cart_data[0x0147])
    {
        // ROM ONLY
        case 0x00:
        case 0x03:
            cart = std::make_shared<GameBoy::ROMOnlyCartridge>(cart_data);
            break;

        // MBC1
        case 0x01:
            cart = std::make_shared<GameBoy::MBC1Cartridge>(cart_data);
            break;

        // MBC3+RAM+BATTERY
        case 0x13:
            cart = std::make_shared<GameBoy::MBC3Cartridge>(cart_data);
            break;
    }

    if (cart)
    {
        bus.cart(cart);
        return cart;
    }

    std::string mbc;

    try
    {
        mbc = Cartridge::types.at(cart_data[0x0147]);
    }
    catch (std::out_of_range&)
    {
        mbc = "unknown";
    }

    throw std::runtime_error
    (fmt::sprintf("Unsupported memory bank controller (%s)", mbc));
}

/// @brief Sets the boot ROM data.
/// @param data If `data` is not empty, a boot ROM is considered to be present.
/// Otherwise, boot ROM functionality will be disabled.
auto System::boot_rom(const std::vector<uint8_t>& data) noexcept -> void
{
    bus.boot_rom(data);

    cpu.reg.bc = 0x0000;
    cpu.reg.de = 0x0000;
    cpu.reg.hl = 0x0000;
    cpu.reg.af = 0x0000;

    cpu.reg.sp = 0x0000;
    cpu.reg.pc = 0x0000;
}

/// @brief Executes one full system step.
/// @return The number of T-cycles taken by the current step.
auto System::step() noexcept -> unsigned int
{
    cpu.step();

    const unsigned int cycles{ bus.cycles };
    bus.cycles = 0;

    return cycles;
}
