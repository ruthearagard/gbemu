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

    // Defines a register pair.
    class RegisterPair
    {
    public:
        RegisterPair(uint8_t& hi, uint8_t& lo) noexcept : m_hi(hi), m_lo(lo)
        { };

        auto value() const noexcept -> uint16_t
        {
            return ((m_hi << 8) | m_lo);
        }

        auto value(const uint16_t v) noexcept -> void
        {
            m_hi = v >> 8;
            m_lo = v & 0x00FF;
        }

        operator uint16_t() { return value(); }
        
        uint8_t& m_hi;
        uint8_t& m_lo;
    };

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
            uint8_t b, c, d, e, h, l, a, f;

            RegisterPair bc;
            RegisterPair de;
            RegisterPair hl;
            RegisterPair af;

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
            DiscardResult,

            // Force the Zero flag to be set to zero.
            ClearZeroFlag,

            Normal
        };

    private:
        inline auto read_next_byte() noexcept -> uint8_t;

        inline auto read_next_word() noexcept -> uint16_t;

        auto stack_pop(RegisterPair& pair) noexcept -> void;

        auto stack_push(const RegisterPair& pair) noexcept -> void;

        auto update_flag(const enum Flag flag,
                         const bool condition_met) noexcept -> void;

        auto bitwise_and(const uint8_t n) noexcept -> void;

        auto bitwise_xor(const uint8_t n) noexcept -> void;

        auto bitwise_or(const uint8_t n) noexcept -> void;

        auto add_sp() noexcept -> uint16_t;

        // Sets the Zero flag to `true` if `value` is 0.
        template<typename T>
        auto set_zero_flag(const T value) noexcept -> void;

        // Sets the Subtract flag to `condition`.
        auto set_subtract_flag(const bool condition) noexcept -> void;

        // Sets the Half Carry flag to `condition`.
        auto set_half_carry_flag(const bool condition) noexcept -> void;

        // Sets the Carry flag to `condition`.
        auto set_carry_flag(const bool condition) noexcept -> void;

        // Handles the `INC r` instruction.
        auto inc(uint8_t r) noexcept -> uint8_t;

        // Handles the `DEC r` instruction.
        auto dec(uint8_t r) noexcept -> uint8_t;

        // Handles the `ADD HL, xx` instruction.
        auto add_hl(const RegisterPair& pair) noexcept -> void;

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

        // Handles the `RST $vector` instruction.
        auto rst(const uint16_t vector) noexcept -> void;

        // Handles the `RLC n` instruction.
        auto rlc(uint8_t n,
                 const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `RRC n` instruction.
        auto rrc(uint8_t n,
                 const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `RR n` instruction.
        auto rr(uint8_t n,
                const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `RL n` instruction.
        auto rl(uint8_t n,
                const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `SLA n` instruction.
        auto sla(uint8_t n) noexcept -> uint8_t;

        // Handles the `SRA n` instruction.
        auto sra(uint8_t n) noexcept -> uint8_t;

        // Handles the `SRL n` instruction.
        auto srl(uint8_t n) noexcept -> uint8_t;

        // Handles the `BIT b, n` instruction.
        auto bit(const unsigned int b, const uint8_t n) -> void;

        // System bus instance
        SystemBus& m_bus;
    };
}