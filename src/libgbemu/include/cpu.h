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

// Required for `std::function<>`.
#include <functional>

namespace GameBoy
{
    // Forward declaration
    class SystemBus;

    using ALUFunc = std::function<uint8_t(uint8_t)>;

    // Defines a Sharp SM83 CPU interpreter.
    class CPU
    {
    public:
        explicit CPU(SystemBus& bus) noexcept;

        // Resets the CPU to the startup state.
        auto reset() noexcept -> void;

        // Executes the next instruction.
        auto step() noexcept -> void;

        // Registers
        struct
        {
            union
            {
                struct
                {
                    uint8_t c;
                    uint8_t b;
                };
                uint16_t bc;
            };

            union
            {
                struct
                {
                    uint8_t e;
                    uint8_t d;
                };
                uint16_t de;
            };

            union
            {
                struct
                {
                    uint8_t l;
                    uint8_t h;
                };
                uint16_t hl;
            };

            union
            {
                struct
                {
                    uint8_t f;
                    uint8_t a;
                };
                uint16_t af;
            };

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

        // Changes how certain functions operate.
        enum OpFlag : unsigned int
        {
            Normal,
            TrulyConditional,
            ExtraDelay
        };

        // ALU function flags
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

        // IME - Interrupt Master Enable Flag (Write Only)
        bool ime;

        bool halted;

    private:
        // Returns a byte from memory referenced by the program counter and
        // increments the program counter.
        inline auto read_next_byte() noexcept -> uint8_t;

        // Returns the next two bytes from memory referenced by the program
        // counter, incrementing the program counter twice.
        inline auto read_next_word() noexcept -> uint16_t;

        inline auto interrupt_check(const Interrupt intr,
                                    const uint8_t ie,
                                    uint8_t& m_if) noexcept -> void;

        // If `condition_met` is true, sets bit `flag` of the Flag Register
        // (F), otherwise unsets it.
        auto update_flag(const enum Flag flag,
                         const bool condition_met) noexcept -> void;

        // Sets the Zero flag to `true` if `value` is 0.
        auto set_zero_flag(const uint8_t value) noexcept -> void;

        // Sets the Zero flag to `condition`.
        auto set_zero_flag(const bool condition) noexcept -> void;

        // Sets the Subtract flag to `condition`.
        auto set_subtract_flag(const bool condition) noexcept -> void;

        // Sets the Half Carry flag to `condition`.
        auto set_half_carry_flag(const bool condition) noexcept -> void;

        // Sets the Carry flag to `condition`.
        auto set_carry_flag(const bool condition) noexcept -> void;

        // Reads the byte stored in memory referenced by register pair HL,
        // calls ALU function `f`, and then stores the result of the ALU
        // operation back into memory referenced by register pair HL.
        auto rw_hl(const ALUFunc& f) noexcept -> void;

        // Performs bitwise operation `op` between the Accumulator (register A)
        // and `n`, and sets the Flag register (F) to `flags_if_zero` if the
        // result is zero, or `flags_if_nonzero` otherwise.
        template<class Operator>
        auto bit_op(const Operator op,
                    const uint8_t n,
                    const unsigned int flags_if_zero,
                    const unsigned int flags_if_nonzero) noexcept -> void;

        // Handles the `INC r` instruction.
        auto inc(uint8_t r) noexcept -> uint8_t;

        // Handles the `DEC r` instruction.
        auto dec(uint8_t r) noexcept -> uint8_t;

        // Handles the `ADD HL, xx` instruction.
        auto add_hl(const uint16_t pair) noexcept -> void;

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
        auto ret(const bool condition_met,
                 const OpFlag flag = OpFlag::Normal) -> void;

        // Pops the stack into register pair `pair`.
        auto stack_pop() noexcept -> uint16_t;

        // Handles the `JP cond, $imm16` instruction.
        auto jp(const bool condition_met) noexcept -> void;

        // Handles the `CALL cond, $imm16` instruction.
        auto call(const bool condition_met) noexcept -> void;

        // Pushes the contents of register pair `pair` onto the stack.
        auto stack_push(const uint16_t pair) noexcept -> void;

        // Handles the `RST $vector` instruction.
        auto rst(const uint16_t vector) noexcept -> void;

        // Performs a special addition operation between the stack pointer (SP)
        // and an immediate byte, and stores the result in `pair`.
        auto add_sp(const OpFlag flag = Normal) noexcept -> uint16_t;

        // Handles the `RLC n` instruction.
        auto rlc(uint8_t n,
                 const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `RRC n` instruction.
        auto rrc(uint8_t n,
                 const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `RL n` instruction.
        auto rl(uint8_t n,
                const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `RR n` instruction.
        auto rr(uint8_t n,
                const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        // Handles the `SLA n` instruction.
        auto sla(uint8_t n) noexcept -> uint8_t;

        // Handles the `SRA n` instruction.
        auto sra(uint8_t n) noexcept -> uint8_t;

        // Handles the `SWAP n` instruction.
        auto swap(uint8_t n) noexcept -> uint8_t;

        // Handles the `SRL n` instruction.
        auto srl(uint8_t n) noexcept -> uint8_t;

        // Handles the `BIT b, n` instruction.
        auto bit(const unsigned int b, const uint8_t n) -> void;

        // Reads the value from memory referenced by register pair HL, performs
        // a bitwise operation `op` against bit `bit`, and stores the result
        // back into memory referenced by register pair `HL`.
        template<class Operator>
        auto bit_hl(const Operator op,
                    const unsigned int bit) noexcept -> void;

        // System bus instance
        SystemBus& m_bus;
    };
}