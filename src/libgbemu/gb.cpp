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

// Required for the `GameBoy::System` class.
#include "gb.h"

using namespace GameBoy;

System::System() noexcept : cpu(bus)
{
    reset();
}

// Resets the system to the startup state.
auto System::reset() noexcept -> void
{
    cpu.reset();
}

// Sets the current cartridge to `cart`.
auto System::cart(const std::shared_ptr<Cartridge>& c) noexcept -> void
{
    bus.cart(c);
}

// Executes one full step and returns the number of cycles taken.
auto System::step() noexcept -> unsigned int
{
    cpu.step();
    return 0;
}