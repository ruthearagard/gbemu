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

#pragma once

#include <cstdint>
#include <functional>

namespace GameBoy
{
    class SystemBus;

    using ALUFunc = std::function<uint8_t(uint8_t)>;

    /// @brief Defines a Sharp SM83 CPU interpreter.
    class CPU final
    {
    public:
        /// @brief Initializes the CPU.
        /// @param bus The current system bus instance.
        explicit CPU(SystemBus& bus) noexcept;

        /// @brief Resets the CPU to the startup state.
        auto reset() noexcept -> void;

        /// @brief Executes the next instruction.
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

        /// @brief Bits of the Flag (F) register.
        enum FlagBit : unsigned int
        {
            Zero      = 1 << 7,
            Subtract  = 1 << 6,
            HalfCarry = 1 << 5,
            Carry     = 1 << 4
        };

        /// @brief Changes how certain instructions operate.
        enum OpFlag : unsigned int
        {
            /// @brief Standard use of the instruction function as written.
            Normal,

            /// @brief The call to the `ret()` method is truly a conditional
            /// operation.
            TrulyConditional,

            /// @brief Take an extra 1 m-cycle delay.
            ExtraDelay
        };

        /// @brief Flags that apply to certain ALU functions.
        enum class ALUFlag
        {
            /// @brief The operation does not take the carry flag into account.
            // This is the default behavior.
            WithoutCarry,

            /// @brief The operation takes the carry flag into account.
            WithCarry,

            /// @brief The flag register is updated, but the result is not
            /// stored into the Accumulator (register A).
            DiscardResult,

            /// @brief Force the Zero flag bit to be set to zero.
            ClearZeroFlag,

            Normal
        };

        /// @brief Interrupt Master Enable Flag
        bool ime;

        bool halted;

    private:
        /// @brief Returns a byte from memory using the program counter as a
        /// memory address, then increments the program counter.
        /// @return The byte from memory.
        auto read_next_byte() noexcept -> uint8_t;

        /// @brief Reads two bytes from memory using the program counter as a
        /// memory address, then increments the program counter twice.
        /// @return The two bytes from memory as a 16-bit integer.
        auto read_next_word() noexcept -> uint16_t;

        auto interrupt_check(const Interrupt intr,
                             const uint8_t ie,
                             uint8_t& m_if) noexcept -> void;

        /// @brief Conditionally sets or resets a bit in the Flag (F) register.
        /// @param bit The bit in question.
        /// @param condition_met If the result of an expression returns `true`,
        /// the bit is set. Otherwise, it is unset.
        auto update_flag_bit(const enum FlagBit bit,
                             const bool condition_met) noexcept -> void;

        /// @brief Updates the Zero flag bit of the Flag register (F) if
        /// `value` is zero, otherwise it is reset.
        /// @param value The value to check against.
        auto set_zero_flag(const uint8_t value) noexcept -> void;

        /// @brief Updates the Zero flag bit of the Flag register (F) based on
        /// `condition`.
        /// @param condition Must be `true` or `false`.
        auto set_zero_flag(const bool condition) noexcept -> void;

        /// @brief Updates the Subtract flag bit of the Flag register (F) based
        /// on `condition`.
        /// @param condition The result of an expression.
        auto set_subtract_flag(const bool condition) noexcept -> void;

        /// @brief Updates the Half Carry flag bit of the Flag register (F)
        /// based on `condition`.
        /// @param condition The result of an expression.
        auto set_half_carry_flag(const bool condition) noexcept -> void;

        /// @brief Updates the Carry flag bit of the Flag register (F) based on
        /// `condition`.
        /// @param condition The result of an expression.
        auto set_carry_flag(const bool condition) noexcept -> void;

        /// @brief Reads a byte from memory using register pair HL as a memory
        /// address, calls an ALU function and stores the result of the ALU
        /// operation back into memory using register pair HL as a memory
        /// address.
        /// @param f The function to call.
        auto rw_hl(const ALUFunc& f) noexcept -> void;

        /// @brief Performs a bitwise operation between the Accumulator
        /// (register A) and a value, sets the Flag register (F) based on
        /// the result of the operation, and stores the the result in the
        /// Accumulator.
        /// @param op The bitwise operation class to use.
        /// @param n The value to use as a parameter to `op`.
        /// @param flags_if_zero The value to set the Flag register to if the
        /// Accumulator is zero.
        /// @param flags_if_nonzero The value to set the Flag register to if
        /// the Accumulator is not zero.
        template<class Operator>
        auto bit_op(const Operator op,
                    const uint8_t n,
                    const unsigned int flags_if_zero,
                    const unsigned int flags_if_nonzero) noexcept -> void;

        /// @brief Handles the INC instruction.
        /// @param r The value to increment.
        /// @return The incremented value.
        auto inc(uint8_t r) noexcept -> uint8_t;

        /// @brief Handles the DEC instruction.
        /// @param r The value to decrement.
        /// @return The decremented value.
        auto dec(uint8_t r) noexcept -> uint8_t;

        /// @brief Handles the ADD HL instruction.
        /// @param pair The register pair to use as an addend.
        auto add_hl(const uint16_t pair) noexcept -> void;

        /// @brief Handles the JR instruction.
        /// @param condition_met The result of an expression.
        auto jr(const bool condition_met) -> void;

        /// @brief Handles an addition instruction.
        /// @param addend The value to use as an addend.
        /// @param flag Must be one of the following:
        /// 
        /// `ALUFlag::WithoutCarry`: ADD instruction (default)
        /// `ALUFlag::WithCarry`: ADC instruction
        auto add(const uint8_t addend,
                 const ALUFlag flag = ALUFlag::WithoutCarry) noexcept -> void;

        /// @brief Handles a subtraction operation.
        /// @param subtrahend The value to use as a subtrahend.
        /// @param flag Must be one of the following:
        /// 
        /// `ALUFlag::WithoutCarry`: SUB instruction (default)
        /// `ALUFlag::WithCarry`: SBC instruction
        /// `ALUFlag::DiscardResult`: CP instruction
        auto sub(const uint8_t subtrahend,
                 const ALUFlag flag = ALUFlag::WithoutCarry) noexcept -> void;

        /// @brief Handles a return from subroutine.
        /// @param condition_met The result of an expression.
        /// @param flag Must be one of the following:
        /// 
        /// `OpFlag::Normal`: RET instruction (default)
        /// `OpFlag::TrulyConditional`: `RET cond` instruction
        auto ret(const bool condition_met,
                 const OpFlag flag = OpFlag::Normal) -> void;

        /// @brief Handles the POP instruction.
        /// @return The result of the stack popping.
        auto stack_pop() noexcept -> uint16_t;

        /// @brief Handles the JP instruction.
        /// @param condition_met The result of an expression.
        auto jp(const bool condition_met) noexcept -> void;

        /// @brief Handles the CALL instruction.
        /// @param condition_met The result of an expression.
        auto call(const bool condition_met) noexcept -> void;

        /// @brief Handles the PUSH instruction.
        /// @param pair The register pair to push onto the stack.
        auto stack_push(const uint16_t pair) noexcept -> void;

        /// @brief Handles the RST instruction.
        /// @param vector The program counter to jump to.
        auto rst(const uint16_t vector) noexcept -> void;

        /// @brief Handles an addition between the stack pointer (SP) and an
        /// immediate byte.
        /// @param flag Must be one of the following:
        /// 
        /// `OpFlag::Normal`: LD HL, SP+$simm8 instruction (default)
        /// `OpFlag::ExtraDelay`: ADD SP, $imm8 instruction
        /// @return The sum of the addition.
        auto add_sp(const OpFlag flag = Normal) noexcept -> uint16_t;

        /// @brief Handles the RLC instruction.
        /// @param n The value to rotate.
        /// @param flag Must be one of the following:
        /// 
        /// `ALUFlag::Normal`: RLC instruction (default)
        /// `ALUFlag::ClearZeroFlag`: RLCA instruction
        /// @return The rotated value.
        auto rlc(uint8_t n,
                 const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        /// @brief Handles the RRC instruction.
        /// @param n The value to rotate.
        /// @param flag Must be one of the following:
        /// 
        /// `ALUFlag::Normal`: RRC instruction (default)
        /// `ALUFlag::ClearZeroFlag`: RRCA instruction
        /// @return The rotated value.
        auto rrc(uint8_t n,
                 const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        /// @brief Handles the RL instruction.
        /// @param n The value to rotate.
        /// @param flag Must be one of the following:
        /// 
        /// `ALUFlag::Normal`: RL instruction (default)
        /// `ALUFlag::ClearZeroFlag`: RLA instruction
        /// @return The rotated value.
        auto rl(uint8_t n,
                const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        /// @brief Handles the RR instruction.
        /// @param n The value to rotate.
        /// @param flag Must be one of the following:
        /// 
        /// `ALUFlag::Normal`: RR instruction (default)
        /// `ALUFlag::ClearZeroFlag`: RRA instruction
        /// @return The rotated value.
        auto rr(uint8_t n,
                const ALUFlag flag = ALUFlag::Normal) noexcept -> uint8_t;

        /// @brief Handles the SLA instruction.
        /// @param n The value to shift.
        /// @return The shifted value.
        auto sla(uint8_t n) noexcept -> uint8_t;

        /// @brief Handles the SRA instruction.
        /// @param n The value to shift.
        /// @return The shifted value.
        auto sra(uint8_t n) noexcept -> uint8_t;

        /// @brief Handles the SWAP instruction.
        /// @param n The value to swap.
        /// @return The swapped value.
        auto swap(uint8_t n) noexcept -> uint8_t;

        /// @brief Handles the SRL instruction.
        /// @param n The value to shift.
        /// @return The shifted value.
        auto srl(uint8_t n) noexcept -> uint8_t;

        /// @brief Handles the BIT instruction.
        /// @param b The bit to test.
        /// @param n The value to test `b` against.
        auto bit(const unsigned int b, const uint8_t n) -> void;

        /// @brief Reads the value from memory using register pair HL as a
        /// memory address, performs a bitwise operation against a bit, and
        /// stores the result back into memory using register pair HL as a
        /// memory address.
        /// @param op The bitwise operator to use.
        /// @param bit The bit to perform the operation on.
        template<class Operator>
        auto bit_hl(const Operator op,
                    const unsigned int bit) noexcept -> void;

        /// @brief System bus instance
        SystemBus& m_bus;
    };
}