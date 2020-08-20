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

            // Accumulator
            uint8_t a;

            // Flag register. Only the 4 upper bits are used; the lower 4 bits
            // are fixed to 0.
            uint8_t f;

            // Program counter
            uint16_t pc;
            
            // Stack pointer
            uint16_t sp;
        } reg;

        // Flag register bits
        enum Flag : unsigned int
        {
            Zero      = 1 << 7,
            Subtract  = 1 << 6,
            HalfCarry = 1 << 5,
            Carry     = 1 << 4
        };

        // ALU function specifiers
        enum class ALUFlag
        {
            // The operation does not take the carry flag into account.
            // This is the default behavior for all ALU operations.
            WithoutCarry,

            // The operation takes the carry flag into account.
            WithCarry,

            // The flag register is updated, but the result is not stored into
            // the Accumulator (register A).
            DiscardResult
        };

        // Returns the value of the `BC` register pair.
        auto bc() const noexcept -> uint16_t;

        // Returns the value of the `DE` register pair.
        auto de() const noexcept -> uint16_t;

        // Returns the value of the `HL` register pair.
        auto hl() const noexcept -> uint16_t;

        // Returns the value of the `AF` register pair.
        auto af() const noexcept -> uint16_t;

    private:
        // Handles the `INC r` instruction.
        auto inc(uint8_t r) noexcept -> uint8_t;

        // Handles the `DEC r` instruction.
        auto dec(uint8_t r) noexcept -> uint8_t;

        // Handles the `JR cond, $branch` instruction.
        auto jr(const bool condition_met) -> void;

        // Handles an addition instruction, based on `flag`:
        //
        // ADD A, `addend` (default): `ALUFlag::WithoutCarry`
        // ADC A, `addend`:           `ALUFlag::WithCarry`
        auto add(const uint8_t addend,
                 const ALUFlag flag = ALUFlag::WithoutCarry) noexcept -> void;

        // Handles a subtraction instruction, based on `flag`:
        //
        // SUB `subtrahend` (default): `ALUFlag::WithoutCarry`
        // SBC A, `subtrahend`:        `ALUFlag::WithCarry`
        // CP `subtrahend`:            `ALUFlag::DiscardResult`
        auto sub(const uint8_t subtrahend,
                 const ALUFlag flag = ALUFlag::WithoutCarry) noexcept -> void;

        // Handles the `RET cond` instruction.
        auto ret(const bool condition_met) -> void;

        // Handles the `JP cond, $imm16` instruction.
        auto jp(const bool condition_met) noexcept -> void;

        // Handles the `CALL cond, $imm16` instruction.
        auto call(const bool condition_met) noexcept -> void;

        // System bus instance
        SystemBus& m_bus;
    };
}