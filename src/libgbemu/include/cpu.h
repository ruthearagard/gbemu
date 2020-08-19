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
// �#ifndef� to guard the contents of header files against multiple inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

// Required for fixed-width integer types (e.g. `uint8_t`).
#include <cstdint>

namespace GameBoy
{
    // Forward declaration
    class SystemBus;

    // Defines a Sharp SM83 CPU interpreter.
    class CPU
    {
    public:
        explicit CPU(SystemBus& bus) noexcept;

        // Resets the CPU to the startup state.
        auto reset() noexcept -> void;

        // Executes the next instruction.
        auto step() noexcept -> void;

        struct
        {
            uint8_t b;
            uint8_t c;

            uint8_t d;
            uint8_t e;

            uint8_t h;
            uint8_t l;

            uint8_t a;

            // Flag register. Only the 4 upper bits are used; the lower 4 bits
            // are fixed to 0.
            uint8_t f;

            // Program counter
            uint16_t pc;
            
            // Stack pointer
            uint16_t sp;
        } reg;

        // Returns the value of the `BC` register pair.
        auto bc() const noexcept -> uint16_t;

        // Returns the value of the `DE` register pair.
        auto de() const noexcept -> uint16_t;

        // Returns the value of the `HL` register pair.
        auto hl() const noexcept -> uint16_t;

        // Returns the value of the `AF` register pair.
        auto af() const noexcept -> uint16_t;

    private:
        // Handles the `JP cond, $imm16` instruction.
        auto jp(const bool condition_met) noexcept -> void;

        // System bus instance
        SystemBus& m_bus;
    };
}