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

// Required for the `GameBoy::CPU` class.
#include "cpu.h"

using namespace GameBoy;

CPU::CPU(SystemBus& bus) noexcept : m_bus(bus)
{
    reset();
}

inline auto CPU::read_next_byte() noexcept -> uint8_t
{
    return m_bus.read(reg.pc++);
}

inline auto CPU::read_next_word() noexcept -> uint16_t
{
    const uint8_t lo{ read_next_byte() };
    const uint8_t hi{ read_next_byte() };

    return (hi << 8) | lo;
}

auto CPU::stack_pop(RegisterPair& pair) noexcept -> void
{
    pair.m_lo = m_bus.read(reg.sp++);
    pair.m_hi = m_bus.read(reg.sp++);
}

auto CPU::stack_push(const RegisterPair& pair) noexcept -> void
{
    m_bus.write(--reg.sp, pair.m_hi);
    m_bus.write(--reg.sp, pair.m_lo);
}

auto CPU::update_flag(const enum Flag flag,
                      const bool condition_met) noexcept -> void
{
    if (condition_met)
    {
        reg.f |= flag;
    }
    else
    {
        reg.f &= ~flag;
    }
}

// Sets the Zero flag to `true` if `value` is 0.
template<typename T>
auto CPU::set_zero_flag(const T value) noexcept -> void
{
    update_flag(Flag::Zero, value == 0);
}

// Sets the Subtract flag.
auto CPU::set_subtract_flag(const bool condition) noexcept -> void
{
    update_flag(Flag::Subtract, condition);
}

// Sets the Half Carry flag to `condition`.
auto CPU::set_half_carry_flag(const bool condition) noexcept -> void
{
    update_flag(Flag::HalfCarry, condition);
}

// Sets the Carry flag to `condition`.
auto CPU::set_carry_flag(const bool condition) noexcept -> void
{
    update_flag(Flag::Carry, condition);
}

// Handles the `INC r` instruction.
auto CPU::inc(uint8_t r) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag((r & 0x0F) == 0x0F);

    r++;

    set_zero_flag(r);
    return r;
}

// Handles the `DEC r` instruction.
auto CPU::dec(uint8_t r) noexcept -> uint8_t
{
    set_subtract_flag(true);
    set_half_carry_flag((r & 0x0F) == 0);

    r--;

    set_zero_flag(r);
    return r;
}

// Handles the `ADD HL, xx` instruction.
auto CPU::add_hl(const RegisterPair& pair) noexcept -> void
{
    set_subtract_flag(false);

    unsigned int result = reg.hl + pair;

    set_half_carry_flag((hl ^ pair ^ result) & 0x1000);
    set_carry_flag(result > 0xFFFF);

    reg.hl = result;
}

// Handles the `JR cond, $branch` instruction.
auto CPU::jr(const bool condition_met) -> void
{
    const int8_t offset{ static_cast<int8_t>(read_next_byte()) };

    if (condition_met)
    {
        reg.pc += offset;
    }
}

// Handles an addition instruction, based on `flag`:
//
// ADD A, `addend` (default): `ALUFlag::WithoutCarry`
// ADC A, `addend`:           `ALUFlag::WithCarry`
auto CPU::add(const uint8_t addend, const ALUFlag flag) noexcept -> void
{
    set_subtract_flag(false);

    unsigned int result = reg.a + addend;

    if (flag == ALUFlag::WithCarry)
    {
        result += (reg.f & Flag::Carry) != 0;
    }

    reg.a = static_cast<uint8_t>(result);

    set_zero_flag(reg.a);
    set_half_carry_flag((reg.a ^ addend ^ result) & 0x10);
    set_carry_flag(result > 0xFF);
}

// Handles a subtraction instruction, based on `flag`:
//
// SUB `subtrahend` (default): `ALUFlag::WithoutCarry`
// SBC A, `subtrahend`:        `ALUFlag::WithCarry`
// CP `subtrahend`:            `ALUFlag::DiscardResult`
auto CPU::sub(const uint8_t subtrahend, const ALUFlag flag) noexcept -> void
{
    set_subtract_flag(true);

    int result{ reg.a - subtrahend };

    if (flag == ALUFlag::WithCarry)
    {
        result -= (reg.f & Flag::Carry) != 0;
    }

    const uint8_t diff{ static_cast<uint8_t>(result) };

    set_zero_flag(diff);
    set_half_carry_flag((reg.a ^ subtrahend ^ result) & 0x10);
    set_carry_flag(result < 0);

    if (flag != ALUFlag::DiscardResult)
    {
        reg.a = diff;
    }
}

// Handles the `RET cond` instruction.
auto CPU::ret(const bool condition_met) -> void
{
    if (condition_met)
    {
        stack_pop(reg.pc);
    }
}

// Handles the `JP cond, $imm16` instruction.
auto CPU::jp(const bool condition_met) noexcept -> void
{
    const uint16_t address{ read_next_word() };

    if (condition_met)
    {
        reg.pc = address;
    }
}

// Handles the `CALL cond, $imm16` instruction.
auto CPU::call(const bool condition_met) noexcept -> void
{
    const uint16_t address{ read_next_word() };

    if (condition_met)
    {
        stack_push(reg.pc);
        reg.pc = address;
    }
}

// Handles the `RST $vector` instruction.
auto CPU::rst(const uint16_t vector) noexcept -> void
{
    stack_push(reg.pc);
    reg.pc = vector;
}

// Handles the `RLC n` instruction.
auto CPU::rlc(uint8_t n, const ALUFlag flag) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag(false);

    set_carry_flag(n & 0x80);

    n = (n << 1) | (n >> 7);

    if (flag == ALUFlag::ClearZeroFlag)
    {
        set_zero_flag(false);
    }
    else
    {
        set_zero_flag(n);
    }
    return n;
}

// Handles the `RRC n` instruction.
auto CPU::rrc(uint8_t n, const ALUFlag flag) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag(false);

    set_carry_flag(n & 1);

    n = (n >> 1) | (n << 7);

    set_zero_flag(n);
    return n;
}

// Handles the `RL n` instruction.
auto CPU::rl(uint8_t n, const ALUFlag flag) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag(false);

    const bool carry{ (reg.f & Flag::Carry) != 0 };

    set_carry_flag(n & 0x80);

    n = (n << 1) | carry;

    set_zero_flag(n);
    return n;
}

// Handles the `RR n` instruction.
auto CPU::rr(uint8_t n, const ALUFlag flag) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag(false);

    const bool old_carry{ (n & 1) != 0 };
    const bool carry{ (reg.f & Flag::Carry) != 0 };

    n = (n >> 1) | (carry << 7);

    set_zero_flag(n);
    set_carry_flag(old_carry);

    return n;
}

// Handles the `SLA n` instruction.
auto CPU::sla(uint8_t n) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag(false);

    set_carry_flag(n & 0x80);

    n <<= 1;

    set_zero_flag(n);
    return n;
}

// Handles the `SRA n` instruction.
auto CPU::sra(uint8_t n) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag(false);

    set_carry_flag(n & 1);

    n = (n >> 1) | (n & 0x80);

    set_zero_flag(n);
    return n;
}

// Handles the `SRL n` instruction.
auto CPU::srl(uint8_t n) noexcept -> uint8_t
{
    set_subtract_flag(false);
    set_half_carry_flag(false);

    const bool carry{ (n & 1) != 0 };

    n >>= 1;

    set_zero_flag(n);
    set_carry_flag(carry);

    return n;
}

// Handles the `BIT b, n` instruction.
auto CPU::bit(const unsigned int b, const uint8_t n) -> void
{
    set_subtract_flag(false);
    set_half_carry_flag(true);
    set_zero_flag(!(n & (1 << b)));
}

// Resets the CPU to the startup state.
auto CPU::reset() noexcept -> void
{
    reg.bc = 0x0013;
    reg.de = 0x00D8;
    reg.hl = 0x014D;
    reg.af = 0x01B0;

    reg.sp = 0xFFFE;
    reg.pc = 0x0100;
}

// Executes the next instruction.
auto CPU::step() noexcept -> void
{
    const uint8_t instruction{ m_bus.read(reg.pc) };

    switch (instruction)
    {
        case 0x00:                                             return; // NOP
        case 0x01: reg.bc = read_next_word();                  return; // LD BC, $imm16
        case 0x02: m_bus.write(reg.bc.value(), reg.a);         return; // LD (BC), A
        case 0x03: reg.bc++;                                   return; // INC BC
        case 0x04: reg.b = inc(reg.b);                         return; // INC B
        case 0x05: reg.b = dec(reg.b);                         return; // DEC B
        case 0x06: reg.b = read_next_byte();                   return; // LD B, $imm8
        case 0x07: reg.a = rlc(reg.a, ALUFlag::ClearZeroFlag); return; // RLCA

        // LD ($imm16), SP
        case 0x08:
        {
            const auto address{ read_next_word() };

            m_bus.write(address,     reg.sp & 0x00FF);
            m_bus.write(address + 1, reg.sp >> 8);

            reg.pc += 3;
            return;
        }

        case 0x09: add_hl(reg.bc);                             return; // ADD HL, BC
        case 0x0A: reg.a = m_bus.read(reg.bc);                 return; // LD A, (BC)
        case 0x0B: reg.bc--;                                   return; // DEC BC
        case 0x0C: reg.c = inc(reg.c);                         return; // INC C
        case 0x0D: reg.c = dec(reg.c);                         return; // DEC C
        case 0x0E: reg.c = read_next_byte();                   return; // LD C, $imm8
        case 0x0F: reg.a = rrc(reg.a, ALUFlag::ClearZeroFlag); return; // RRCA
        case 0x11: reg.de = read_next_word();                  return; // LD DE, $imm16
        case 0x12: m_bus.write(reg.de.value(), reg.a);         return; // LD (DE), A
        case 0x13: reg.de++;                                   return; // INC DE
        case 0x14: reg.d = inc(reg.d);                         return; // INC D
        case 0x15: reg.d = dec(reg.d);                         return; // DEC D
        case 0x16: reg.d = read_next_byte();                   return; // LD D, $imm8
        case 0x17: reg.a = rl(reg.a, ALUFlag::ClearZeroFlag);  return; // RLA
        case 0x18: jr(true);                                   return; // JR $branch
        case 0x19: add_hl(reg.de);                             return; // ADD HL, DE
        case 0x1A: reg.a = m_bus.read(reg.de);                 return; // LD A, (DE)
        case 0x1B: reg.de--;                                   return; // DEC DE
        case 0x1C: reg.e = inc(reg.e);                         return; // INC E
        case 0x1D: reg.e = dec(reg.e);                         return; // DEC E
        case 0x1E: reg.e = read_next_byte();                   return; // LD E, $imm8
        case 0x1F: reg.a = rr(reg.a, ALUFlag::ClearZeroFlag);  return; // RRA
        case 0x20: jr(!(reg.f & Flag::Zero));                  return; // JR NZ, $branch
        case 0x21: reg.hl = read_next_word();                  return; // LD HL, $imm16
        case 0x22: m_bus.write(reg.hl, reg.a); reg.hl++;       return; // LD (HL+), A
        case 0x23: reg.hl++;                                   return; // INC HL
        case 0x24: reg.h = inc(reg.h);                         return; // INC H
        case 0x25: reg.h = dec(reg.h);                         return; // DEC H
        case 0x26: reg.h = read_next_byte();                   return; // LD H, $imm8

        // DAA
        //
        // XXX: NOT MY CODE. Taken from https://bit.ly/3j24lzB under the terms
        // of the MIT license.
        case 0x27:
        {
            uint8_t adjust{ 0 };

            // See if we had a carry/borrow for the low nibble in the last
            // operation.
            if (reg.f & Flag::HalfCarry)
            {
                // Yes, we have to adjust it.
                adjust |= 0x06;
            }

            // See if we had a carry/borrow for the high nibble in the last
            // operation.
            if (reg.f & Flag::Carry)
            {
                // Yes, we have to adjust it.
                adjust |= 0x60;
            }

            if (reg.f & Flag::Subtract)
            {
                // If the operation was a subtraction we're done since we
                // can never end up in the A-F range by substracting
                // without generating a (half)carry.
                reg.a -= adjust;
            }
            else
            {
                // Additions are a bit more tricky because we might have
                // to adjust even if we haven't overflowed (and no carry
                // is present). For instance: 0x8 + 0x4 -> 0xc.
                if ((reg.a & 0x0F) > 0x09)
                {
                    adjust |= 0x06;
                }

                if (reg.a > 0x99)
                {
                    adjust |= 0x60;
                }
                reg.a += adjust;
            }

            set_zero_flag(reg.a);
            set_carry_flag(adjust & 0x60);
            set_half_carry_flag(false);

            return;
        }

        case 0x28: jr(reg.f & Flag::Zero);               return; // JR Z, $branch
        case 0x29: add_hl(reg.hl);                       return; // ADD HL, HL
        case 0x2A: reg.a = m_bus.read(reg.hl); reg.hl++; return; // LD A, (HL+)
        case 0x2B: reg.hl--;                             return; // DEC HL
        case 0x2C: reg.l = inc(reg.l);                   return; // INC L
        case 0x2D: reg.l = dec(reg.l);                   return; // DEC L
        case 0x2E: reg.l = read_next_byte();             return; // LD L, $imm8

        // CPL
        case 0x2F:
            reg.a = ~reg.a;

            set_subtract_flag(true);
            set_half_carry_flag(true);

            return;

        case 0x30: jr(!(reg.f & Flag::Carry));            return; // JR NC, $branch
        case 0x31: reg.sp = read_next_word();             return; // LD SP, $imm16
        case 0x32: m_bus.write(reg.hl, reg.a); reg.hl--;  return; // LD (HL-), A
        case 0x33: reg.sp++;                              return; // INC SP

        // INC (HL)
        case 0x34:
        {
            const uint16_t m_hl{ hl() };

            uint8_t data{ m_bus.read(m_hl) };

            data = inc(data);
            m_bus.write(m_hl, data);

            reg.pc++;
            return;
        }

        // DEC (HL)
        case 0x35:
        {
            const uint16_t m_hl{ hl() };

            uint8_t data{ m_bus.read(m_hl) };

            data = dec(data);
            m_bus.write(m_hl, data);

            reg.pc++;
            return;
        }

        case 0x36: m_bus.write(reg.hl, read_next_byte()); return; // LD (HL), $imm8

        // SCF
        case 0x37:
            set_carry_flag(true);
            set_subtract_flag(false);
            set_half_carry_flag(false);

            return;

        case 0x38: jr(reg.f & Flag::Carry); return;
        case 0x39: add_hl(reg.sp); return;

        // LD A, (HL-)
        case 0x3A:
        {
            uint16_t m_hl{ hl() };

            reg.a = m_bus.read(m_hl--);

            reg.h = m_hl >> 8;
            reg.l = m_hl & 0x00FF;

            reg.pc++;
            return;
        }

        case 0x3B: reg.sp--; return;
        case 0x3C: reg.a = inc(reg.a); return;
        case 0x3D: reg.a = dec(reg.a); return;
        case 0x3E: reg.a = read_next_byte(); return;

        // CCF
        case 0x3F:
            set_subtract_flag(false);
            set_half_carry_flag(false);
            set_carry_flag(!(reg.f & Flag::Carry));

            return;

        case 0x40:                                                      return; // LD B, B
        case 0x41: reg.b = reg.c;                                       return; // LD B, C
        case 0x42: reg.b = reg.d;                                       return; // LD B, D
        case 0x43: reg.b = reg.e;                                       return; // LD B, E
        case 0x44: reg.b = reg.h;                                       return; // LD B, H
        case 0x45: reg.b = reg.l;                                       return; // LD B, L
        case 0x46: reg.b = m_bus.read(reg.hl);                          return; // LD B, (HL)
        case 0x47: reg.b = reg.a;                                       return; // LD B, A
        case 0x48: reg.c = reg.b;                                       return; // LD C, B
        case 0x49:                                                      return; // LD C, C
        case 0x4A: reg.c = reg.d;                                       return; // LD C, D
        case 0x4B: reg.c = reg.e;                                       return; // LD C, E
        case 0x4C: reg.c = reg.h;                                       return; // LD C, H
        case 0x4D: reg.c = reg.l;                                       return; // LD C, L
        case 0x4E: reg.c = m_bus.read(reg.hl);                          return; // LD C, (HL)
        case 0x4F: reg.c = reg.a;                                       return; // LD C, A
        case 0x50: reg.d = reg.b;                                       return; // LD D, B
        case 0x51: reg.d = reg.c;                                       return; // LD D, C
        case 0x52:                                                      return; // LD D, D
        case 0x53: reg.d = reg.e;                                       return; // LD D, E
        case 0x54: reg.d = reg.h;                                       return; // LD D, H
        case 0x55: reg.d = reg.l;                                       return; // LD D, L
        case 0x56: reg.d = m_bus.read(reg.hl);                          return; // LD D, (HL)
        case 0x57: reg.d = reg.a;                                       return; // LD D, A
        case 0x58: reg.e = reg.b;                                       return; // LD E, B
        case 0x59: reg.e = reg.c;                                       return; // LD E, C
        case 0x5A: reg.e = reg.d;                                       return; // LD E, D
        case 0x5B:                                                      return; // LD E, E
        case 0x5C: reg.e = reg.h;                                       return; // LD E, H
        case 0x5D: reg.e = reg.l;                                       return; // LD E, L
        case 0x5E: reg.e = m_bus.read(reg.hl);                          return; // LD E, (HL)
        case 0x5F: reg.e = reg.a;                                       return; // LD E, A
        case 0x60: reg.h = reg.b;                                       return; // LD H, B
        case 0x61: reg.h = reg.c;                                       return; // LD H, C
        case 0x62: reg.h = reg.d;                                       return; // LD H, D
        case 0x63: reg.h = reg.e;                                       return; // LD H, E
        case 0x64:                                                      return; // LD H, H
        case 0x65: reg.h = reg.l;                                       return; // LD H, L
        case 0x66: reg.h = m_bus.read(reg.hl);                          return; // LD H, (HL)
        case 0x67: reg.h = reg.a;                                       return; // LD H, A
        case 0x68: reg.l = reg.b;                                       return; // LD L, B
        case 0x69: reg.l = reg.c;                                       return; // LD L, C
        case 0x6A: reg.l = reg.d;                                       return; // LD L, D
        case 0x6B: reg.l = reg.e;                                       return; // LD L, E
        case 0x6C: reg.l = reg.h;                                       return; // LD L, H
        case 0x6D:                                                      return; // LD L, L
        case 0x6E: reg.l = m_bus.read(reg.hl);                          return; // LD L, (HL)
        case 0x6F: reg.l = reg.a;                                       return; // LD L, A
        case 0x70: m_bus.write(reg.hl, reg.b);                          return; // LD (HL), B
        case 0x71: m_bus.write(reg.hl, reg.c);                          return; // LD (HL), C
        case 0x72: m_bus.write(reg.hl, reg.d);                          return; // LD (HL), D
        case 0x73: m_bus.write(reg.hl, reg.e);                          return; // LD (HL), E
        case 0x74: m_bus.write(reg.hl, reg.h);                          return; // LD (HL), H
        case 0x75: m_bus.write(reg.hl, reg.l);                          return; // LD (HL), L
        case 0x77: m_bus.write(reg.hl, reg.a);                          return; // LD (HL), A
        case 0x78: reg.a = reg.b;                                       return; // LD A, B
        case 0x79: reg.a = reg.c;                                       return; // LD A, C
        case 0x7A: reg.a = reg.d;                                       return; // LD A, D
        case 0x7B: reg.a = reg.e;                                       return; // LD A, E
        case 0x7C: reg.a = reg.h;                                       return; // LD A, H
        case 0x7D: reg.a = reg.l;                                       return; // LD A, L
        case 0x7E: reg.a = m_bus.read(reg.hl);                          return; // LD A, (HL)
        case 0x7F:                                                      return; // LD A, A
        case 0x80: add(reg.b);                                          return; // ADD A, B
        case 0x81: add(reg.c);                                          return; // ADD A, C
        case 0x82: add(reg.d);                                          return; // ADD A, D
        case 0x83: add(reg.e);                                          return; // ADD A, E
        case 0x84: add(reg.h);                                          return; // ADD A, H
        case 0x85: add(reg.l);                                          return; // ADD A, L
        case 0x86: add(m_bus.read(reg.hl));                             return; // ADD A, (HL)
        case 0x87: add(reg.a);                                          return; // ADD A, A
        case 0x88: add(reg.b, ALUFlag::WithCarry);                      return; // ADC A, B
        case 0x89: add(reg.c, ALUFlag::WithCarry);                      return; // ADC A, C
        case 0x8A: add(reg.d, ALUFlag::WithCarry);                      return; // ADC A, D
        case 0x8B: add(reg.e, ALUFlag::WithCarry);                      return; // ADC A, E
        case 0x8C: add(reg.h, ALUFlag::WithCarry);                      return; // ADC A, H
        case 0x8D: add(reg.l, ALUFlag::WithCarry);                      return; // ADC A, L
        case 0x8E: add(m_bus.read(reg.hl), ALUFlag::WithCarry);         return; // ADC A, (HL)
        case 0x8F: add(reg.a, ALUFlag::WithCarry);                      return; // ADC A, A
        case 0x90: sub(reg.b);                                          return; // SUB B
        case 0x91: sub(reg.c);                                          return; // SUB C
        case 0x92: sub(reg.d);                                          return; // SUB D
        case 0x93: sub(reg.e);                                          return; // SUB E
        case 0x94: sub(reg.h);                                          return; // SUB H
        case 0x95: sub(reg.l);                                          return; // SUB L
        case 0x96: sub(m_bus.read(reg.hl));                             return; // SUB (HL)
        case 0x97: sub(reg.a);                                          return; // SUB A
        case 0x98: sub(reg.b, ALUFlag::WithCarry);                      return; // SBC A, B
        case 0x99: sub(reg.c, ALUFlag::WithCarry);                      return; // SBC A, C
        case 0x9A: sub(reg.d, ALUFlag::WithCarry);                      return; // SBC A, D
        case 0x9B: sub(reg.e, ALUFlag::WithCarry);                      return; // SBC A, E
        case 0x9C: sub(reg.h, ALUFlag::WithCarry);                      return; // SBC A, H
        case 0x9D: sub(reg.l, ALUFlag::WithCarry);                      return; // SBC A, L
        case 0x9E: sub(m_bus.read(reg.hl.value()), ALUFlag::WithCarry); return; // SBC A, (HL)
        case 0x9F: sub(reg.a, ALUFlag::WithCarry);                      return; // SBC A, A
        case 0xA0: bitwise_and(reg.b);                                  return; // AND B
        case 0xA1: bitwise_and(reg.c);                                  return; // AND C
        case 0xA2: bitwise_and(reg.d);                                  return; // AND D
        case 0xA3: bitwise_and(reg.e);                                  return; // AND E
        case 0xA4: bitwise_and(reg.h);                                  return; // AND H
        case 0xA5: bitwise_and(reg.l);                                  return; // AND L
        case 0xA6: bitwise_and(m_bus.read(reg.hl));                     return; // AND (HL)
        case 0xA7: bitwise_and(reg.a);                                  return; // AND A
        case 0xA8: bitwise_xor(reg.b);                                  return; // XOR B
        case 0xA9: bitwise_xor(reg.c);                                  return; // XOR C
        case 0xAA: bitwise_xor(reg.d);                                  return; // XOR D
        case 0xAB: bitwise_xor(reg.e);                                  return; // XOR E
        case 0xAC: bitwise_xor(reg.h);                                  return; // XOR H
        case 0xAD: bitwise_xor(reg.l);                                  return; // XOR L
        case 0xAE: bitwise_xor(m_bus.read(reg.hl));                     return; // XOR (HL)
        case 0xAF: bitwise_xor(reg.a);                                  return; // XOR A
        case 0xB0: bitwise_or(reg.b);                                   return; // OR B
        case 0xB1: bitwise_or(reg.c);                                   return; // OR C
        case 0xB2: bitwise_or(reg.d);                                   return; // OR D
        case 0xB3: bitwise_or(reg.e);                                   return; // OR E
        case 0xB4: bitwise_or(reg.h);                                   return; // OR H
        case 0xB5: bitwise_or(reg.l);                                   return; // OR L
        case 0xB6: bitwise_or(m_bus.read(reg.hl));                      return; // OR (HL)
        case 0xB7: bitwise_or(reg.a);                                   return; // OR A
        case 0xB8: sub(reg.b, ALUFlag::DiscardResult);                  return; // CP B
        case 0xB9: sub(reg.c, ALUFlag::DiscardResult);                  return; // CP C
        case 0xBA: sub(reg.d, ALUFlag::DiscardResult);                  return; // CP D
        case 0xBB: sub(reg.e, ALUFlag::DiscardResult);                  return; // CP E
        case 0xBC: sub(reg.h, ALUFlag::DiscardResult);                  return; // CP H
        case 0xBD: sub(reg.l, ALUFlag::DiscardResult);                  return; // CP L
        case 0xBE: sub(m_bus.read(reg.hl), ALUFlag::DiscardResult);     return; // CP (HL)
        case 0xBF: sub(reg.a, ALUFlag::DiscardResult);                  return; // CP A
        case 0xC0: ret(!(reg.f & Flag::Zero));                          return; // RET NZ
        case 0xC1: stack_pop(reg.bc);                                   return; // POP BC
        case 0xC2: jp(!(reg.f & Flag::Zero));                           return; // JP NZ, $imm16
        case 0xC3: jp(true);                                            return; // JP $imm16
        case 0xC4: call(!(reg.f & Flag::Zero));                         return; // CALL NZ, $imm16
        case 0xC5: stack_push(reg.bc);                                  return; // PUSH BC
        case 0xC6: add(read_next_byte());                               return; // ADD A, $imm8
        case 0xC7: rst(0x0000);                                         return; // RST $0000
        case 0xC8: ret(reg.f & Flag::Zero);                             return; // RET Z
        case 0xC9: ret(true);                                           return; // RET
        case 0xCA: jp(reg.f & Flag::Zero);                              return; // JP Z, $imm16

        // CB-prefixed instruction
        case 0xCB:
            switch (read_next_byte())
            {
                case 0x00: reg.b = rlc(reg.b); return; // RLC B
                case 0x01: reg.c = rlc(reg.c); return; // RLC C
                case 0x02: reg.d = rlc(reg.d); return; // RLC D
                case 0x03: reg.e = rlc(reg.e); return; // RLC E
                case 0x04: reg.h = rlc(reg.h); return; // RLC H
                case 0x05: reg.l = rlc(reg.l); return; // RLC L

                // RLC (HL)
                case 0x06:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = rlc(data);
                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                case 0x07: reg.a = rlc(reg.a); return; // RLC A

                case 0x08: reg.b = rrc(reg.b); return; // RRC B
                case 0x09: reg.c = rrc(reg.c); return; // RRC C
                case 0x0A: reg.d = rrc(reg.d); return; // RRC D
                case 0x0B: reg.e = rrc(reg.e); return; // RRC E
                case 0x0C: reg.h = rrc(reg.h); return; // RRC H
                case 0x0D: reg.l = rrc(reg.l); return; // RRC L

                // RRC (HL)
                case 0x0E:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = rrc(data);
                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                case 0x0F: reg.a = rrc(reg.a); return; // RRC A

                case 0x10: reg.b = rl(reg.b); return; // RL B
                case 0x11: reg.c = rl(reg.c); return; // RL C
                case 0x12: reg.d = rl(reg.d); return; // RL D
                case 0x13: reg.e = rl(reg.e); return; // RL E
                case 0x14: reg.h = rl(reg.h); return; // RL H
                case 0x15: reg.l = rl(reg.l); return; // RL L

                // RL (HL)
                case 0x16:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = rl(data);
                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                case 0x17: reg.a = rl(reg.a); return; // RL A

                case 0x18: reg.b = rr(reg.b); return; // RR B
                case 0x19: reg.c = rr(reg.c); return; // RR C
                case 0x1A: reg.d = rr(reg.d); return; // RR D
                case 0x1B: reg.e = rr(reg.e); return; // RR E
                case 0x1C: reg.h = rr(reg.h); return; // RR H
                case 0x1D: reg.l = rr(reg.l); return; // RR L

                // RR (HL)
                case 0x1E:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = rr(data);
                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                case 0x1F: reg.a = rr(reg.a); return; // RR A

                case 0x20: reg.b = sla(reg.b); return; // SLA B
                case 0x21: reg.c = sla(reg.c); return; // SLA C
                case 0x22: reg.d = sla(reg.d); return; // SLA D
                case 0x23: reg.e = sla(reg.e); return; // SLA E
                case 0x24: reg.h = sla(reg.h); return; // SLA H
                case 0x25: reg.l = sla(reg.l); return; // SLA L

                // SLA (HL)
                case 0x26:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = sla(data);
                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                case 0x27: reg.a = sla(reg.a); return; // SLA A
 
                case 0x28: reg.b = sra(reg.b); return; // SRA B
                case 0x29: reg.c = sra(reg.c); return; // SRA C
                case 0x2A: reg.d = sra(reg.d); return; // SRA D
                case 0x2B: reg.e = sra(reg.e); return; // SRA E
                case 0x2C: reg.h = sra(reg.h); return; // SRA H
                case 0x2D: reg.l = sra(reg.l); return; // SRA L

                // SRA (HL)
                case 0x2E:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = sra(data);
                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                // SRA A
                case 0x2F: reg.a = sra(reg.a); return;

                case 0x30: reg.b = swap(reg.b); return; // SWAP B
                case 0x31: reg.c = swap(reg.c); return; // SWAP C
                case 0x32: reg.d = swap(reg.d); return; // SWAP D
                case 0x33: reg.e = swap(reg.e); return; // SWAP E
                case 0x34: reg.h = swap(reg.h); return; // SWAP H
                case 0x35: reg.l = swap(reg.l); return; // SWAP L

                // SWAP (HL)
                case 0x36:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = ((data & 0x0F) << 4) | (data >> 4);
                    reg.f = (data == 0) ? 0x80 : 0x00;

                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                case 0x37: reg.a = swap(reg.a); return; // SWAP A

                case 0x38: reg.b = srl(reg.b); return; // SRL B
                case 0x39: reg.c = srl(reg.c); return; // SRL C
                case 0x3A: reg.d = srl(reg.d); return; // SRL D
                case 0x3B: reg.e = srl(reg.e); return; // SRL E
                case 0x3C: reg.h = srl(reg.h); return; // SRL H
                case 0x3D: reg.l = srl(reg.l); return; // SRL L

                // SRL (HL)
                case 0x3E:
                {
                    const uint16_t m_hl{ hl() };

                    uint8_t data{ m_bus.read(m_hl) };

                    data = srl(data);
                    m_bus.write(m_hl, data);

                    reg.pc += 2;
                    return;
                }

                case 0x3F: reg.a = srl(reg.a); return; // SRL A

                case 0x40: bit(0, reg.b);                      return; // BIT 0, B
                case 0x41: bit(0, reg.c);                      return; // BIT 0, C
                case 0x42: bit(0, reg.d);                      return; // BIT 0, D
                case 0x43: bit(0, reg.e);                      return; // BIT 0, E
                case 0x44: bit(0, reg.h);                      return; // BIT 0, H
                case 0x45: bit(0, reg.l);                      return; // BIT 0, L
                case 0x46: bit(0, m_bus.read(reg.hl.value())); return; // BIT 0, (HL)
                case 0x47: bit(0, reg.a);                      return; // BIT 0, A
                case 0x48: bit(1, reg.b);                      return; // BIT 1, B
                case 0x49: bit(1, reg.c);                      return; // BIT 1, C
                case 0x4A: bit(1, reg.d);                      return; // BIT 1, D
                case 0x4B: bit(1, reg.e);                      return; // BIT 1, E
                case 0x4C: bit(1, reg.h);                      return; // BIT 1, H
                case 0x4D: bit(1, reg.l);                      return; // BIT 1, L
                case 0x4E: bit(1, m_bus.read(reg.hl.value())); return; // BIT 1, (HL)
                case 0x4F: bit(1, reg.a);                      return; // BIT 1, A
                case 0x50: bit(2, reg.b);                      return; // BIT 2, B
                case 0x51: bit(2, reg.c);                      return; // BIT 2, C
                case 0x52: bit(2, reg.d);                      return; // BIT 2, D
                case 0x53: bit(2, reg.e);                      return; // BIT 2, E
                case 0x54: bit(2, reg.h);                      return; // BIT 2, H
                case 0x55: bit(2, reg.l);                      return; // BIT 2, L
                case 0x56: bit(2, m_bus.read(reg.hl.value())); return; // BIT 2, (HL)
                case 0x57: bit(2, reg.a);                      return; // BIT 2, A
                case 0x58: bit(3, reg.b);                      return; // BIT 3, B
                case 0x59: bit(3, reg.c);                      return; // BIT 3, C
                case 0x5A: bit(3, reg.d);                      return; // BIT 3, D
                case 0x5B: bit(3, reg.e);                      return; // BIT 3, E
                case 0x5C: bit(3, reg.h);                      return; // BIT 3, H
                case 0x5D: bit(3, reg.l);                      return; // BIT 3, L
                case 0x5E: bit(3, m_bus.read(reg.hl.value())); return; // BIT 3, (HL)
                case 0x5F: bit(3, reg.a);                      return; // BIT 3, A
                case 0x60: bit(4, reg.b);                      return; // BIT 4, B
                case 0x61: bit(4, reg.c);                      return; // BIT 4, C
                case 0x62: bit(4, reg.d);                      return; // BIT 4, D
                case 0x63: bit(4, reg.e);                      return; // BIT 4, E
                case 0x64: bit(4, reg.h);                      return; // BIT 4, H
                case 0x65: bit(4, reg.l);                      return; // BIT 4, L
                case 0x66: bit(4, m_bus.read(reg.hl.value())); return; // BIT 4, (HL)
                case 0x67: bit(4, reg.a);                      return; // BIT 4, A
                case 0x68: bit(5, reg.b);                      return; // BIT 5, B
                case 0x69: bit(5, reg.c);                      return; // BIT 5, C
                case 0x6A: bit(5, reg.d);                      return; // BIT 5, D
                case 0x6B: bit(5, reg.e);                      return; // BIT 5, E
                case 0x6C: bit(5, reg.h);                      return; // BIT 5, H
                case 0x6D: bit(5, reg.l);                      return; // BIT 5, L
                case 0x6E: bit(5, m_bus.read(reg.hl.value())); return; // BIT 5, (HL)
                case 0x6F: bit(5, reg.a);                      return; // BIT 5, A
                case 0x70: bit(6, reg.b);                      return; // BIT 6, B
                case 0x71: bit(6, reg.c);                      return; // BIT 6, C
                case 0x72: bit(6, reg.d);                      return; // BIT 6, D
                case 0x73: bit(6, reg.e);                      return; // BIT 6, E
                case 0x74: bit(6, reg.h);                      return; // BIT 6, H
                case 0x75: bit(6, reg.l);                      return; // BIT 6, L
                case 0x76: bit(6, m_bus.read(reg.hl.value())); return; // BIT 6, (HL)
                case 0x77: bit(6, reg.a);                      return; // BIT 6, A
                case 0x78: bit(7, reg.b);                      return; // BIT 7, B
                case 0x79: bit(7, reg.c);                      return; // BIT 7, C
                case 0x7A: bit(7, reg.d);                      return; // BIT 7, D
                case 0x7B: bit(7, reg.e);                      return; // BIT 7, E
                case 0x7C: bit(7, reg.h);                      return; // BIT 7, H
                case 0x7D: bit(7, reg.l);                      return; // BIT 7, L
                case 0x7E: bit(7, m_bus.read(reg.hl.value())); return; // BIT 7, (HL)
                case 0x7F: bit(7, reg.a);                      return; // BIT 7, A
                case 0x80: reg.b = res(0, reg.b);              return; // RES 0, B
                case 0x81: reg.c = res(0, reg.c);              return; // RES 0, C
                case 0x82: reg.d = res(0, reg.d);              return; // RES 0, D
                case 0x83: reg.e = res(0, reg.e);              return; // RES 0, E
                case 0x84: reg.h = res(0, reg.h);              return; // RES 0, H
                case 0x85: reg.l = res(0, reg.l);              return; // RES 0, L
                case 0x86: res_hl(0);                          return; // RES 0, (HL)
                case 0x87: reg.a = res(0, reg.a);              return; // RES 0, A
                case 0x88: reg.b = res(1, reg.b);              return; // RES 1, B
                case 0x89: reg.c = res(1, reg.c);              return; // RES 1, C
                case 0x8A: reg.d = res(1, reg.d);              return; // RES 1, D
                case 0x8B: reg.e = res(1, reg.e);              return; // RES 1, E
                case 0x8C: reg.h = res(1, reg.h);              return; // RES 1, H
                case 0x8D: reg.l = res(1, reg.l);              return; // RES 1, L
                case 0x8E: res_hl(1);                          return; // RES 1, (HL)
                case 0x8F: reg.a = res(1, reg.a);              return; // RES 1, A
                case 0x90: reg.b = res(2, reg.b);              return; // RES 2, B
                case 0x91: reg.c = res(2, reg.c);              return; // RES 2, C
                case 0x92: reg.d = res(2, reg.d);              return; // RES 2, D
                case 0x93: reg.e = res(2, reg.e);              return; // RES 2, E
                case 0x94: reg.h = res(2, reg.h);              return; // RES 2, H
                case 0x95: reg.l = res(2, reg.l);              return; // RES 2, L
                case 0x96: res_hl(2);                          return; // RES 2, (HL)
                case 0x97: reg.a = res(2, reg.a);              return; // RES 2, A
                case 0x98: reg.b = res(3, reg.b);              return; // RES 3, B
                case 0x99: reg.c = res(3, reg.c);              return; // RES 3, C
                case 0x9A: reg.d = res(3, reg.d);              return; // RES 3, D
                case 0x9B: reg.e = res(3, reg.e);              return; // RES 3, E
                case 0x9C: reg.h = res(3, reg.h);              return; // RES 3, H
                case 0x9D: reg.l = res(3, reg.l);              return; // RES 3, L
                case 0x9E: res_hl(3);                          return; // RES 3, (HL)
                case 0x9F: reg.a = res(3, reg.a);              return; // RES 3, A
                case 0xA0: reg.b = res(4, reg.b);              return; // RES 4, B
                case 0xA1: reg.c = res(4, reg.c);              return; // RES 4, C
                case 0xA2: reg.d = res(4, reg.d);              return; // RES 4, D
                case 0xA3: reg.e = res(4, reg.e);              return; // RES 4, E
                case 0xA4: reg.h = res(4, reg.h);              return; // RES 4, H
                case 0xA5: reg.l = res(4, reg.l);              return; // RES 4, L
                case 0xA6: res_hl(4);                          return; // RES 4, (HL)
                case 0xA7: reg.a = res(4, reg.a);              return; // RES 4, A
                case 0xA8: reg.b = res(5, reg.b);              return; // RES 5, B
                case 0xA9: reg.c = res(5, reg.c);              return; // RES 5, C
                case 0xAA: reg.d = res(5, reg.d);              return; // RES 5, D
                case 0xAB: reg.e = res(5, reg.e);              return; // RES 5, E
                case 0xAC: reg.h = res(5, reg.h);              return; // RES 5, H
                case 0xAD: reg.l = res(5, reg.l);              return; // RES 5, L
                case 0xAE: res_hl(5);                          return; // RES 5, (HL)
                case 0xAF: reg.a = res(5, reg.a);              return; // RES 5, A
                case 0xB0: reg.b = res(6, reg.b);              return; // RES 6, B
                case 0xB1: reg.c = res(6, reg.c);              return; // RES 6, C
                case 0xB2: reg.d = res(6, reg.d);              return; // RES 6, D
                case 0xB3: reg.e = res(6, reg.e);              return; // RES 6, E
                case 0xB4: reg.h = res(6, reg.h);              return; // RES 6, H
                case 0xB5: reg.l = res(6, reg.l);              return; // RES 6, L
                case 0xB6: res_hl(6);                          return; // RES 6, (HL)
                case 0xB7: reg.a = res(6, reg.a);              return; // RES 6, A
                case 0xB8: reg.b = res(7, reg.b);              return; // RES 7, B
                case 0xB9: reg.c = res(7, reg.c);              return; // RES 7, C
                case 0xBA: reg.d = res(7, reg.d);              return; // RES 7, D
                case 0xBB: reg.e = res(7, reg.e);              return; // RES 7, E
                case 0xBC: reg.h = res(7, reg.h);              return; // RES 7, H
                case 0xBD: reg.l = res(7, reg.l);              return; // RES 7, L
                case 0xBE: res_hl(7);                          return; // RES 7, (HL)
                case 0xBF: reg.a = res(7, reg.a);              return; // RES 7, A
                case 0xC0: reg.b = set(0, reg.b);              return; // SET 0, B
                case 0xC1: reg.c = set(0, reg.c);              return; // SET 0, C
                case 0xC2: reg.d = set(0, reg.d);              return; // SET 0, D
                case 0xC3: reg.e = set(0, reg.e);              return; // SET 0, E
                case 0xC4: reg.h = set(0, reg.h);              return; // SET 0, H
                case 0xC5: reg.l = set(0, reg.l);              return; // SET 0, L
                case 0xC6: set_hl(0);                          return; // SET 0, (HL)
                case 0xC7: reg.a = set(0, reg.a);              return; // SET 0, A
                case 0xC8: reg.b = set(1, reg.b);              return; // SET 1, B
                case 0xC9: reg.c = set(1, reg.c);              return; // SET 1, C
                case 0xCA: reg.d = set(1, reg.d);              return; // SET 1, D
                case 0xCB: reg.e = set(1, reg.e);              return; // SET 1, E
                case 0xCC: reg.h = set(1, reg.h);              return; // SET 1, H
                case 0xCD: reg.l = set(1, reg.l);              return; // SET 1, L
                case 0xCE: set_hl(1);                          return; // SET 1, (HL)
                case 0xCF: reg.a = set(1, reg.a);              return; // SET 1, A
                case 0xD0: reg.b = set(2, reg.b);              return; // SET 2, B
                case 0xD1: reg.c = set(2, reg.c);              return; // SET 2, C
                case 0xD2: reg.d = set(2, reg.d);              return; // SET 2, D
                case 0xD3: reg.e = set(2, reg.e);              return; // SET 2, E
                case 0xD4: reg.h = set(2, reg.h);              return; // SET 2, H
                case 0xD5: reg.l = set(2, reg.l);              return; // SET 2, L
                case 0xD6: set_hl(2);                          return; // SET 2, (HL)
                case 0xD7: reg.a = set(2, reg.a);              return; // SET 2, A
                case 0xD8: reg.b = set(3, reg.b);              return; // SET 3, B
                case 0xD9: reg.c = set(3, reg.c);              return; // SET 3, C
                case 0xDA: reg.d = set(3, reg.d);              return; // SET 3, D
                case 0xDB: reg.e = set(3, reg.e);              return; // SET 3, E
                case 0xDC: reg.h = set(3, reg.h);              return; // SET 3, H
                case 0xDD: reg.l = set(3, reg.l);              return; // SET 3, L
                case 0xDE: set_hl(3);                          return; // SET 3, (HL)
                case 0xDF: reg.a = set(3, reg.a);              return; // SET 3, A
                case 0xE0: reg.b = set(4, reg.b);              return; // SET 4, B
                case 0xE1: reg.c = set(4, reg.c);              return; // SET 4, C
                case 0xE2: reg.d = set(4, reg.d);              return; // SET 4, D
                case 0xE3: reg.e = set(4, reg.e);              return; // SET 4, E
                case 0xE4: reg.h = set(4, reg.h);              return; // SET 4, H
                case 0xE5: reg.l = set(4, reg.l);              return; // SET 4, L
                case 0xE6: set_hl(4);                          return; // SET 4, (HL)
                case 0xE7: reg.a = set(4, reg.a);              return; // SET 4, A
                case 0xE8: reg.b = set(5, reg.b);              return; // SET 5, B
                case 0xE9: reg.c = set(5, reg.c);              return; // SET 5, C
                case 0xEA: reg.d = set(5, reg.d);              return; // SET 5, D
                case 0xEB: reg.e = set(5, reg.e);              return; // SET 5, E
                case 0xEC: reg.h = set(5, reg.h);              return; // SET 5, H
                case 0xED: reg.l = set(5, reg.l);              return; // SET 5, L
                case 0xEE: set_hl(5);                          return; // SET 5, (HL)
                case 0xEF: reg.a = set(5, reg.a);              return; // SET 5, A
                case 0xF0: reg.b = set(6, reg.b);              return; // SET 6, B
                case 0xF1: reg.c = set(6, reg.c);              return; // SET 6, C
                case 0xF2: reg.d = set(6, reg.d);              return; // SET 6, D
                case 0xF3: reg.e = set(6, reg.e);              return; // SET 6, E
                case 0xF4: reg.h = set(6, reg.h);              return; // SET 6, H
                case 0xF5: reg.l = set(6, reg.l);              return; // SET 6, L
                case 0xF6: set_hl(6);                          return; // SET 6, (HL)
                case 0xF7: reg.a = set(6, reg.a);              return; // SET 6, A
                case 0xF8: reg.b = set(7, reg.b);              return; // SET 7, B
                case 0xF9: reg.c = set(7, reg.c);              return; // SET 7, C
                case 0xFA: reg.d = set(7, reg.d);              return; // SET 7, D
                case 0xFB: reg.e = set(7, reg.e);              return; // SET 7, E
                case 0xFC: reg.h = set(7, reg.h);              return; // SET 7, H
                case 0xFD: reg.l = set(7, reg.l);              return; // SET 7, L
                case 0xFE: set_hl(7);                          return; // SET 7, (HL)
                case 0xFF: reg.a = set(7, reg.a);              return; // SET 7, A
            }

        case 0xCC: call(reg.f & Flag::Zero);                         return; // CALL Z, $imm16
        case 0xCD: call(true);                                       return; // CALL $imm16
        case 0xCE: add(read_next_byte(), ALUFlag::WithCarry);        return; // ADC A, $imm8
        case 0xCF: rst(0x0008);                                      return; // RST $08
        case 0xD0: ret(!(reg.f & Flag::Carry));                      return; // RET NC
        case 0xD1: stack_pop(reg.de);                                return; // POP DE
        case 0xD2: jp(!(reg.f & Flag::Carry));                       return; // JP NC, $imm16
        case 0xD4: call(!(reg.f & Flag::Carry));                     return; // CALL NC, $imm16
        case 0xD5: stack_push(reg.de);                               return; // PUSH DE
        case 0xD6: sub(read_next_byte());                            return; // SUB $imm8
        case 0xD7: rst(0x0010);                                      return; // RST $0010
        case 0xD8: ret(reg.f & Flag::Carry);                         return; // RET C
        case 0xD9: ret(true);                                        return; // RETI
        case 0xDA: jp(reg.f & Flag::Carry);                          return; // JP C, $imm16
        case 0xDC: call(reg.f & Flag::Carry);                        return; // CALL C, $imm16
        case 0xDE: sub(read_next_byte(), ALUFlag::WithCarry);        return; // SBC A, $imm8
        case 0xDF: rst(0x0018);                                      return; // RST $0018
        case 0xE0: m_bus.write(0xFF00 + read_next_byte(), reg.a);    return; // LDH ($imm8), A
        case 0xE1: stack_pop(reg.hl);                                return; // POP HL
        case 0xE2: m_bus.write(0xFF00 + reg.c, reg.a);               return; // LD (C), A
        case 0xE5: stack_push(reg.hl);                               return; // PUSH HL
        case 0xE6: bitwise_and(read_next_byte());                    return; // AND $imm8
        case 0xE7: rst(0x0020);                                      return; // RST $0020

        // ADD SP, $simm8
        case 0xE8:
        {
            set_zero_flag(false);
            set_subtract_flag(false);

            const int8_t imm{ static_cast<int8_t>(read_next_byte()) };

            const int sum = reg.sp + imm;

            set_half_carry_flag((reg.sp ^ imm ^ sum) & 0x10);
            set_carry_flag((reg.sp ^ imm ^ sum) & 0x100);

            reg.sp = static_cast<uint16_t>(sum);
            return;
        }

        case 0xE9: reg.pc = reg.hl.value(); return;
        case 0xEA: m_bus.write(read_next_word(), reg.a); return;
        case 0xEE: bitwise_xor(read_next_byte()); return;
        case 0xEF: rst(0x0028); return;
        case 0xF0: m_bus.read(0xFF00 + read_next_byte()); return;
        case 0xF1: reg.af = stack_pop(); return;
        case 0xF2: reg.a = m_bus.read(0xFF00 + reg.c); return;
        case 0xF3: return;
        case 0xF5: stack_push(reg.af); return;
        case 0xF6: bitwise_or(read_next_byte()); return;
        case 0xF7: rst(0x0030); return;

        // LD HL, SP+r8
        case 0xF8:
        {
            set_zero_flag(false);
            set_subtract_flag(false);

            const int8_t imm{ static_cast<int8_t>(read_next_byte()) };

            const int sum = reg.sp + imm;

            set_half_carry_flag((reg.sp ^ imm ^ sum) & 0x10);
            set_carry_flag((reg.sp ^ imm ^ sum) & 0x100);

            reg.hl = static_cast<uint16_t>(sum);
            return;
        }

        case 0xF9: reg.sp = reg.hl.value();                       return;
        case 0xFA: reg.a = m_bus.read(read_next_word());          return;
        case 0xFE: sub(read_next_byte(), ALUFlag::DiscardResult); return;
        case 0xFF: rst(0x0038);                                   return;

        default:
            __debugbreak();
    }
}