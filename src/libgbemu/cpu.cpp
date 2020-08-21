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

// Returns the value of the `BC` register pair.
auto CPU::bc() const noexcept -> uint16_t
{
    return (reg.b << 8) | reg.c;
}

// Returns the value of the `DE` register pair.
auto CPU::de() const noexcept -> uint16_t
{
    return (reg.d << 8) | reg.e;
}

// Returns the value of the `HL` register pair.
auto CPU::hl() const noexcept -> uint16_t
{
    return (reg.h << 8) | reg.l;
}

// Returns the value of the `AF` register pair.
auto CPU::af() const noexcept -> uint16_t
{
    return (reg.a << 8) | reg.f;
}

CPU::CPU(SystemBus& bus) noexcept : m_bus(bus)
{
    reset();
}

// Handles the `INC r` instruction.
auto CPU::inc(uint8_t r) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;

    if ((r & 0x0F) == 0x0F)
    {
        reg.f |= Flag::HalfCarry;
    }
    else
    {
        reg.f &= ~Flag::HalfCarry;
    }

    r++;

    if (r == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
    return r;
}

// Handles the `DEC r` instruction.
auto CPU::dec(uint8_t r) noexcept -> uint8_t
{
    reg.f |= Flag::Subtract;

    if ((r & 0x0F) == 0)
    {
        reg.f |= Flag::HalfCarry;
    }
    else
    {
        reg.f &= ~Flag::HalfCarry;
    }

    r--;

    if (r == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
    return r;
}

// Handles the `ADD HL, xx` instruction.
auto CPU::add_hl(const uint16_t pair) noexcept -> void
{
    reg.f &= ~Flag::Subtract;

    const uint16_t m_hl{ hl() };

    unsigned int result = m_hl + pair;

    if ((m_hl ^ pair ^ result) & 0x1000)
    {
        reg.f |= Flag::HalfCarry;
    }
    else
    {
        reg.f &= ~Flag::HalfCarry;
    }

    if (result > 0xFFFF)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    const uint16_t sum{ static_cast<uint16_t>(result) };

    reg.h = sum >> 8;
    reg.l = sum & 0x00FF;
}

// Handles the `JR cond, $branch` instruction.
auto CPU::jr(const bool condition_met) -> void
{
    if (condition_met)
    {
        const int8_t offset{ static_cast<int8_t>(m_bus.read(reg.pc + 1)) };
        reg.pc += offset + 2;
    }
    else
    {
        reg.pc += 2;
    }
}

// Handles an addition instruction, based on `flag`:
//
// ADD A, `addend` (default): `ALUFlag::WithoutCarry`
// ADC A, `addend`:           `ALUFlag::WithCarry`
auto CPU::add(const uint8_t addend, const ALUFlag flag) noexcept -> void
{
    reg.f &= ~Flag::Subtract;

    unsigned int result = reg.a + addend;

    if (flag == ALUFlag::WithCarry)
    {
        const bool carry{ (reg.f & Flag::Carry) != 0 };
        result += carry;
    }

    const uint8_t sum = static_cast<uint8_t>(result);

    if (sum == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }

    if ((reg.a ^ addend ^ result) & 0x10)
    {
        reg.f |= Flag::HalfCarry;
    }
    else
    {
        reg.f &= ~Flag::HalfCarry;
    }

    if (result > 0xFF)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }
    reg.a = sum;
}

// Handles a subtraction instruction, based on `flag`:
//
// SUB `subtrahend` (default): `ALUFlag::WithoutCarry`
// SBC A, `subtrahend`:        `ALUFlag::WithCarry`
// CP `subtrahend`:            `ALUFlag::DiscardResult`
auto CPU::sub(const uint8_t subtrahend, const ALUFlag flag) noexcept -> void
{
    reg.f |= Flag::Subtract;

    int diff{ reg.a - subtrahend };

    if (flag == ALUFlag::WithCarry)
    {
        const bool carry{ (reg.f & Flag::Carry) != 0 };
        diff -= carry;
    }

    const uint8_t d_diff{ static_cast<uint8_t>(diff) };

    if (d_diff == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }

    if ((reg.a ^ subtrahend ^ diff) & 0x10)
    {
        reg.f |= Flag::HalfCarry;
    }
    else
    {
        reg.f &= ~Flag::HalfCarry;
    }

    if (diff < 0)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    if (flag != ALUFlag::DiscardResult)
    {
        reg.a = d_diff;
    }
}

// Handles the `RET cond` instruction.
auto CPU::ret(const bool condition_met) -> void
{
    if (condition_met)
    {
        const uint8_t lo{ m_bus.read(reg.sp++) };
        const uint8_t hi{ m_bus.read(reg.sp++) };

        reg.pc = (hi << 8) | lo;
    }
    else
    {
        reg.pc++;
    }
}

// Handles the `JP cond, $imm16` instruction.
auto CPU::jp(const bool condition_met) noexcept -> void
{
    if (condition_met)
    {
        const uint8_t lo{ m_bus.read(reg.pc + 1) };
        const uint8_t hi{ m_bus.read(reg.pc + 2) };

        reg.pc = (hi << 8) | lo;
    }
    else
    {
        reg.pc += 3;
    }
}

// Handles the `CALL cond, $imm16` instruction.
auto CPU::call(const bool condition_met) noexcept -> void
{
    if (condition_met)
    {
        const uint8_t lo{ m_bus.read(reg.pc + 1) };
        const uint8_t hi{ m_bus.read(reg.pc + 2) };

        reg.pc += 3;

        m_bus.write(--reg.sp, reg.pc >> 8);
        m_bus.write(--reg.sp, reg.pc & 0x00FF);

        reg.pc = (hi << 8) | lo;
    }
    else
    {
        reg.pc += 3;
    }
}

// Handles the `RST $vector` instruction.
auto CPU::rst(const uint16_t vector) noexcept -> void
{
    reg.pc++;

    m_bus.write(--reg.sp, reg.pc >> 8);
    m_bus.write(--reg.sp, reg.pc & 0x00FF);

    reg.pc = vector;
}

// Handles the `RLC n` instruction.
auto CPU::rlc(uint8_t n) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;
    reg.f &= ~Flag::HalfCarry;

    if (n & 0x80)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    n = (n << 1) | (n >> 7);

    if (n == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
    return n;
}

// Handles the `RRC n` instruction.
auto CPU::rrc(uint8_t n) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;
    reg.f &= ~Flag::HalfCarry;

    if (n & 1)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    n = (n >> 1) | (n << 7);

    if (n == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
    return n;
}

// Handles the `RL n` instruction.
auto CPU::rl(uint8_t n) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;
    reg.f &= ~Flag::HalfCarry;

    const bool carry{ (reg.f & Flag::Carry) != 0 };

    if (n & 0x80)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    n = (n << 1) | carry;

    if (n == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
    return n;
}

// Handles the `RR n` instruction.
auto CPU::rr(uint8_t n) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;
    reg.f &= ~Flag::HalfCarry;

    const bool old_carry{ (n & 1) != 0 };
    const bool carry{ (reg.f & Flag::Carry) != 0 };

    n = (n >> 1) | (carry << 7);

    if (n == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }

    if (old_carry)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }
    return n;
}

// Handles the `SLA n` instruction.
auto CPU::sla(uint8_t n) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;
    reg.f &= ~Flag::HalfCarry;

    if (n & 0x80)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    n <<= 1;

    if (n == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
    return n;
}

// Handles the `SRA n` instruction.
auto CPU::sra(uint8_t n) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;
    reg.f &= ~Flag::HalfCarry;

    if (n & 1)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    n = (n >> 1) | (n & 0x80);

    if (n == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
    return n;
}

// Handles the `SRL n` instruction.
auto CPU::srl(uint8_t n) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;
    reg.f &= ~Flag::HalfCarry;

    const bool carry{ (n & 1) != 0 };

    n >>= 1;

    if (n == 0)
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }

    if (carry)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }
    return n;
}

// Handles the `BIT b, n` instruction.
auto CPU::bit(const unsigned int b, const uint8_t n) -> void
{
    reg.f &= ~Flag::Subtract;
    reg.f |= Flag::HalfCarry;

    if (!(n & (1 << b)))
    {
        reg.f |= Flag::Zero;
    }
    else
    {
        reg.f &= ~Flag::Zero;
    }
}

// Resets the CPU to the startup state.
auto CPU::reset() noexcept -> void
{
    reg.b = 0x00;
    reg.c = 0x13;

    reg.d = 0x00;
    reg.e = 0xD8;

    reg.h = 0x01;
    reg.l = 0x4D;

    reg.a = 0x01;
    reg.f = 0xB0;

    reg.sp = 0xFFFE;
    reg.pc = 0x0100;
}

// Executes the next instruction.
auto CPU::step() noexcept -> void
{
    const uint8_t instruction{ m_bus.read(reg.pc) };

    switch (instruction)
    {
        // NOP
        case 0x00:
            reg.pc++;
            return;

        // LD BC, $imm16
        case 0x01:
            reg.c = m_bus.read(reg.pc + 1);
            reg.b = m_bus.read(reg.pc + 2);

            reg.pc += 3;
            return;

        // INC BC
        case 0x03:
        {
            uint16_t m_bc{ bc() };

            m_bc++;

            reg.b = m_bc >> 8;
            reg.c = m_bc & 0x00FF;

            reg.pc++;
            return;
        }

        // INC B
        case 0x04:
            reg.b = inc(reg.b);
            reg.pc++;

            return;

        // DEC B
        case 0x05:
            reg.b = dec(reg.b);
            reg.pc++;

            return;

        // LD B, $imm8
        case 0x06:
            reg.b = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // RLCA
        case 0x07:
            reg.a = rlc(reg.a);
            reg.f &= ~Flag::Zero;

            reg.pc++;
            return;

        // LD ($imm16), SP
        case 0x08:
        {
            const uint8_t lo{ m_bus.read(reg.pc + 1) };
            const uint8_t hi{ m_bus.read(reg.pc + 2) };

            const uint16_t address = (hi << 8) | lo;

            m_bus.write(address,     reg.sp & 0x00FF);
            m_bus.write(address + 1, reg.sp >> 8);

            reg.pc += 3;
            return;
        }

        // ADD HL, BC
        case 0x09:
            add_hl(bc());
            reg.pc++;

            return;

        // DEC BC
        case 0x0B:
        {
            uint16_t m_bc{ bc() };

            m_bc--;

            reg.b = m_bc >> 8;
            reg.c = m_bc & 0x00FF;

            reg.pc++;
            return;
        }

        // INC C
        case 0x0C:
            reg.c = inc(reg.c);
            reg.pc++;

            return;

        // DEC C
        case 0x0D:
            reg.c = dec(reg.c);
            reg.pc++;

            return;

        // LD C, $imm8
        case 0x0E:
            reg.c = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // RRCA
        case 0x0F:
            reg.a = rrc(reg.a);
            reg.f &= ~Flag::Zero;

            reg.pc++;
            return;

        // LD DE, $imm16
        case 0x11:
            reg.e = m_bus.read(reg.pc + 1);
            reg.d = m_bus.read(reg.pc + 2);

            reg.pc += 3;
            return;

        // LD (DE), A
        case 0x12:
            m_bus.write(de(), reg.a);
            reg.pc++;

            return;

        // INC DE
        case 0x13:
        {
            uint16_t m_de{ de() };

            m_de++;

            reg.d = m_de >> 8;
            reg.e = m_de & 0x00FF;

            reg.pc++;
            return;
        }

        // INC D
        case 0x14:
            reg.d = inc(reg.d);
            reg.pc++;

            return;

        // DEC D
        case 0x15:
            reg.d = dec(reg.d);
            reg.pc++;

            return;

        // LD D, $imm8
        case 0x16:
            reg.d = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // RLA
        case 0x17:
            reg.a = rl(reg.a);
            reg.f &= ~Flag::Zero;

            reg.pc++;
            return;

        // JR $branch
        case 0x18:
            jr(true);
            return;

        // ADD HL, DE
        case 0x19:
            add_hl(de());
            reg.pc++;

            return;

        // LD A, (DE)
        case 0x1A:
            reg.a = m_bus.read(de());
            reg.pc++;

            return;

        // DEC DE
        case 0x1B:
        {
            uint16_t m_de{ de() };

            m_de--;

            reg.d = m_de >> 8;
            reg.e = m_de & 0x00FF;

            reg.pc++;
            return;
        }

        // INC E
        case 0x1C:
            reg.e = inc(reg.e);
            reg.pc++;

            return;

        // DEC E
        case 0x1D:
            reg.e = dec(reg.e);
            reg.pc++;

            return;

        // LD E, $imm8
        case 0x1E:
            reg.e = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // RRA
        case 0x1F:
            reg.a = rr(reg.a);
            reg.f &= ~Flag::Zero;

            reg.pc++;
            return;

        // JR NZ, $branch
        case 0x20:
            jr(!(reg.f & Flag::Zero));
            return;

        // LD HL, $imm16
        case 0x21:
            reg.l = m_bus.read(reg.pc + 1);
            reg.h = m_bus.read(reg.pc + 2);

            reg.pc += 3;
            return;

        // LD (HL+), A
        case 0x22:
        {
            uint16_t m_hl{ hl() };

            m_bus.write(m_hl++, reg.a);

            reg.h = m_hl >> 8;
            reg.l = m_hl & 0x00FF;

            reg.pc++;
            return;
        }

        // INC HL
        case 0x23:
        {
            uint16_t m_hl{ hl() };

            m_hl++;

            reg.h = m_hl >> 8;
            reg.l = m_hl & 0x00FF;

            reg.pc++;
            return;
        }

        // INC H
        case 0x24:
            reg.h = inc(reg.h);
            reg.pc++;

            return;

        // DEC H
        case 0x25:
            reg.h = dec(reg.h);
            reg.pc++;

            return;

        // LD H, $imm8
        case 0x26:
            reg.h = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // DAA
        case 0x27:
            reg.pc++;
            return;

        // JR Z, $branch
        case 0x28:
            jr(reg.f & Flag::Zero);
            return;

        // ADD HL, HL
        case 0x29:
            add_hl(hl());
            reg.pc++;

            return;

        // LD A, (HL+)
        case 0x2A:
        {
            uint16_t m_hl{ hl() };

            reg.a = m_bus.read(m_hl++);

            reg.h = m_hl >> 8;
            reg.l = m_hl & 0x00FF;

            reg.pc++;
            return;
        }

        // DEC HL
        case 0x2B:
        {
            uint16_t m_hl{ hl() };

            m_hl--;

            reg.h = m_hl >> 8;
            reg.l = m_hl & 0x00FF;

            reg.pc++;
            return;
        }

        // INC L
        case 0x2C:
            reg.l = inc(reg.l);
            reg.pc++;

            return;

        // DEC L
        case 0x2D:
            reg.l = dec(reg.l);
            reg.pc++;

            return;

        // LD L, $imm8
        case 0x2E:
            reg.l = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // CPL
        case 0x2F:
            reg.a = ~reg.a;

            reg.f |= Flag::Subtract;
            reg.f |= Flag::HalfCarry;

            reg.pc++;
            return;

        // JR NC, $branch
        case 0x30:
            jr(!(reg.f & Flag::Carry));
            return;

        // LD SP, $imm16
        case 0x31:
        {
            const uint8_t lo{ m_bus.read(reg.pc + 1) };
            const uint8_t hi{ m_bus.read(reg.pc + 2) };

            reg.sp = (hi << 8) | lo;
            reg.pc += 3;

            return;
        }

        // LD (HL-), A
        case 0x32:
        {
            uint16_t m_hl{ hl() };

            m_bus.write(m_hl--, reg.a);

            reg.h = m_hl >> 8;
            reg.l = m_hl & 0x00FF;

            reg.pc++;
            return;
        }

        // INC SP
        case 0x33:
            reg.sp++;
            reg.pc++;

            return;

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

        // LD (HL), $imm8
        case 0x36:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            m_bus.write(hl(), imm);
            reg.pc += 2;

            return;
        }

        // SCF
        case 0x37:
            reg.f |= Flag::Carry;
            reg.f &= ~Flag::Subtract;
            reg.f &= ~Flag::HalfCarry;

            reg.pc++;
            return;

        // JR C, $branch
        case 0x38:
            jr(reg.f & Flag::Carry);
            return;

        // ADD HL, SP
        case 0x39:
            add_hl(reg.sp);
            reg.pc++;

            return;

        // DEC SP
        case 0x3B:
            reg.sp--;
            reg.pc++;

            return;

        // INC A
        case 0x3C:
            reg.a = inc(reg.a);
            reg.pc++;

            return;

        // DEC A
        case 0x3D:
            reg.a = dec(reg.a);
            reg.pc++;

            return;

        // LD A, $imm8
        case 0x3E:
            reg.a = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // CCF
        case 0x3F:
            reg.f &= ~Flag::Subtract;
            reg.f &= ~Flag::HalfCarry;

            if (reg.f & Flag::Carry)
            {
                reg.f &= ~Flag::Carry;
            }
            else
            {
                reg.f |= Flag::Carry;
            }
            reg.pc++;
            return;

        // LD B, B
        case 0x40:
            reg.pc++;
            return;

        // LD B, C
        case 0x41:
            reg.b = reg.c;
            reg.pc++;

            return;

        // LD B, D
        case 0x42:
            reg.b = reg.d;
            reg.pc++;

            return;

        // LD B, E
        case 0x43:
            reg.b = reg.e;
            reg.pc++;

            return;

        // LD B, H
        case 0x44:
            reg.b = reg.h;
            reg.pc++;

            return;

        // LD B, L
        case 0x45:
            reg.b = reg.l;
            reg.pc++;

            return;

        // LD B, (HL)
        case 0x46:
            reg.b = m_bus.read(hl());
            reg.pc++;

            return;

        // LD B, A
        case 0x47:
            reg.b = reg.a;
            reg.pc++;

            return;

        // LD C, B
        case 0x48:
            reg.c = reg.b;
            reg.pc++;

            return;

        // LD C, C
        case 0x49:
            reg.pc++;
            return;

        // LD C, D
        case 0x4A:
            reg.c = reg.d;
            reg.pc++;

            return;

        // LD C, E
        case 0x4B:
            reg.c = reg.e;
            reg.pc++;

            return;

        // LD C, H
        case 0x4C:
            reg.c = reg.h;
            reg.pc++;

            return;

        // LD C, L
        case 0x4D:
            reg.c = reg.l;
            reg.pc++;

            return;

        // LD C, (HL)
        case 0x4E:
            reg.c = m_bus.read(hl());
            reg.pc++;

            return;

        // LD C, A
        case 0x4F:
            reg.c = reg.a;
            reg.pc++;

            return;

        // LD D, B
        case 0x50:
            reg.d = reg.b;
            reg.pc++;

            return;

        // LD D, C
        case 0x51:
            reg.d = reg.c;
            reg.pc++;

            return;

        // LD D, D
        case 0x52:
            reg.pc++;
            return;

        // LD D, E
        case 0x53:
            reg.d = reg.e;
            reg.pc++;

            return;

        // LD D, H
        case 0x54:
            reg.d = reg.h;
            reg.pc++;

            return;

        // LD D, L
        case 0x55:
            reg.d = reg.l;
            reg.pc++;

            return;

        // LD D, (HL)
        case 0x56:
            reg.d = m_bus.read(hl());
            reg.pc++;

            return;

        // LD D, A
        case 0x57:
            reg.d = reg.a;
            reg.pc++;

            return;

        // LD E, B
        case 0x58:
            reg.e = reg.b;
            reg.pc++;

            return;

        // LD E, C
        case 0x59:
            reg.e = reg.c;
            reg.pc++;

            return;

        // LD E, D
        case 0x5A:
            reg.e = reg.d;
            reg.pc++;

            return;

        // LD E, E
        case 0x5B:
            reg.pc++;
            return;

        // LD E, H
        case 0x5C:
            reg.e = reg.h;
            reg.pc++;

            return;

        // LD E, L
        case 0x5D:
            reg.e = reg.l;
            reg.pc++;

            return;

        // LD E, (HL)
        case 0x5E:
            reg.e = m_bus.read(hl());
            reg.pc++;

            return;

        // LD E, A
        case 0x5F:
            reg.e = reg.a;
            reg.pc++;

            return;

        // LD H, B
        case 0x60:
            reg.h = reg.b;
            reg.pc++;

            return;

        // LD H, C
        case 0x61:
            reg.h = reg.c;
            reg.pc++;

            return;

        // LD H, D
        case 0x62:
            reg.h = reg.d;
            reg.pc++;

            return;

        // LD H, E
        case 0x63:
            reg.h = reg.e;
            reg.pc++;

            return;

        // LD H, H
        case 0x64:
            reg.pc++;
            return;

        // LD H, L
        case 0x65:
            reg.h = reg.l;
            reg.pc++;

            return;

        // LD H, (HL)
        case 0x66:
            reg.h = m_bus.read(hl());
            reg.pc++;

            return;

        // LD H, A
        case 0x67:
            reg.h = reg.a;
            reg.pc++;

            return;

        // LD L, B
        case 0x68:
            reg.l = reg.b;
            reg.pc++;

            return;

        // LD L, C
        case 0x69:
            reg.l = reg.c;
            reg.pc++;

            return;

        // LD L, D
        case 0x6A:
            reg.l = reg.d;
            reg.pc++;

            return;

        // LD L, E
        case 0x6B:
            reg.l = reg.e;
            reg.pc++;

            return;

        // LD L, H
        case 0x6C:
            reg.l = reg.h;
            reg.pc++;

            return;

        // LD L, L
        case 0x6D:
            reg.pc++;
            return;

        // LD L, (HL)
        case 0x6E:
            reg.l = m_bus.read(hl());
            reg.pc++;

            return;

        // LD L, A
        case 0x6F:
            reg.l = reg.a;
            reg.pc++;

            return;

        // LD (HL), B
        case 0x70:
            m_bus.write(hl(), reg.b);
            reg.pc++;

            return;

        // LD (HL), C
        case 0x71:
            m_bus.write(hl(), reg.c);
            reg.pc++;

            return;

        // LD (HL), D
        case 0x72:
            m_bus.write(hl(), reg.d);
            reg.pc++;

            return;

        // LD (HL), E
        case 0x73:
            m_bus.write(hl(), reg.e);
            reg.pc++;

            return;

        // LD (HL), H
        case 0x74:
            m_bus.write(hl(), reg.h);
            reg.pc++;

            return;

        // LD (HL), L
        case 0x75:
            m_bus.write(hl(), reg.l);
            reg.pc++;

            return;

        // LD (HL), A
        case 0x77:
            m_bus.write(hl(), reg.a);
            reg.pc++;

            return;

        // LD A, B
        case 0x78:
            reg.a = reg.b;
            reg.pc++;

            return;

        // LD A, C
        case 0x79:
            reg.a = reg.c;
            reg.pc++;

            return;

        // LD A, D
        case 0x7A:
            reg.a = reg.d;
            reg.pc++;

            return;

        // LD A, E
        case 0x7B:
            reg.a = reg.e;
            reg.pc++;

            return;

        // LD A, H
        case 0x7C:
            reg.a = reg.h;
            reg.pc++;

            return;

        // LD A, L
        case 0x7D:
            reg.a = reg.l;
            reg.pc++;

            return;

        // LD A, (HL)
        case 0x7E:
            reg.a = m_bus.read(hl());
            reg.pc++;

            return;

        // LD A, A
        case 0x7F:
            reg.pc++;
            return;

        // ADD A, B
        case 0x80:
            add(reg.b);
            reg.pc++;

            return;

        // ADD A, C
        case 0x81:
            add(reg.c);
            reg.pc++;

            return;

        // ADD A, D
        case 0x82:
            add(reg.d);
            reg.pc++;

            return;

        // ADD A, E
        case 0x83:
            add(reg.e);
            reg.pc++;

            return;

        // ADD A, H
        case 0x84:
            add(reg.h);
            reg.pc++;

            return;

        // ADD A, L
        case 0x85:
            add(reg.l);
            reg.pc++;

            return;

        // ADD A, A
        case 0x87:
            add(reg.a);
            reg.pc++;

            return;

        // ADC A, B
        case 0x88:
            add(reg.b, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // ADC A, C
        case 0x89:
            add(reg.c, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // ADC A, D
        case 0x8A:
            add(reg.d, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // ADC A, E
        case 0x8B:
            add(reg.e, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // ADC A, H
        case 0x8C:
            add(reg.h, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // ADC A, L
        case 0x8D:
            add(reg.l, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // ADC A, A
        case 0x8F:
            add(reg.a, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // SUB B
        case 0x90:
            sub(reg.b);
            reg.pc++;

            return;

        // SUB C
        case 0x91:
            sub(reg.c);
            reg.pc++;

            return;

        // SUB D
        case 0x92:
            sub(reg.d);
            reg.pc++;

            return;

        // SUB E
        case 0x93:
            sub(reg.e);
            reg.pc++;

            return;

        // SUB H
        case 0x94:
            sub(reg.h);
            reg.pc++;

            return;

        // SUB L
        case 0x95:
            sub(reg.l);
            reg.pc++;

            return;

        // SUB A
        case 0x97:
            sub(reg.a);
            reg.pc++;

            return;

        // SBC A, B
        case 0x98:
            sub(reg.b, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // SBC A, C
        case 0x99:
            sub(reg.c, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // SBC A, D
        case 0x9A:
            sub(reg.d, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // SBC A, E
        case 0x9B:
            sub(reg.e, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // SBC A, H
        case 0x9C:
            sub(reg.h, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // SBC A, L
        case 0x9D:
            sub(reg.l, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // SBC A, A
        case 0x9F:
            sub(reg.a, ALUFlag::WithCarry);
            reg.pc++;

            return;

        // AND B
        case 0xA0:
            reg.a &= reg.b;
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;

            reg.pc++;
            return;

        // AND C
        case 0xA1:
            reg.a &= reg.c;
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;

            reg.pc++;
            return;

        // AND D
        case 0xA2:
            reg.a &= reg.d;
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;

            reg.pc++;
            return;

        // AND E
        case 0xA3:
            reg.a &= reg.e;
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;

            reg.pc++;
            return;

        // AND H
        case 0xA4:
            reg.a &= reg.h;
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;

            reg.pc++;
            return;

        // AND L
        case 0xA5:
            reg.a &= reg.l;
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;

            reg.pc++;
            return;

        // AND A
        case 0xA7:
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;
            reg.pc++;

            return;

        // XOR B
        case 0xA8:
            reg.a ^= reg.b;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // XOR C
        case 0xA9:
            reg.a ^= reg.c;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // XOR D
        case 0xAA:
            reg.a ^= reg.d;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // XOR E
        case 0xAB:
            reg.a ^= reg.e;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // XOR H
        case 0xAC:
            reg.a ^= reg.h;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // XOR L
        case 0xAD:
            reg.a ^= reg.l;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // XOR (HL)
        case 0xAE:
        {
            const uint8_t data{ m_bus.read(hl()) };

            reg.a ^= data;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;
        }

        // XOR A
        case 0xAF:
            reg.a ^= reg.a;
            reg.f = 0x80;

            reg.pc++;
            return;

        // OR B
        case 0xB0:
            reg.a |= reg.b;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // OR C
        case 0xB1:
            reg.a |= reg.c;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // OR D
        case 0xB2:
            reg.a |= reg.d;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // OR E
        case 0xB3:
            reg.a |= reg.e;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // OR H
        case 0xB4:
            reg.a |= reg.h;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // OR L
        case 0xB5:
            reg.a |= reg.l;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // OR (HL)
        case 0xB6:
        {
            const uint8_t data{ m_bus.read(hl()) };

            reg.a |= data;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;
        }

        // OR A
        case 0xB7:
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // CP B
        case 0xB8:
            sub(reg.b, ALUFlag::DiscardResult);
            reg.pc++;

            return;

        // CP C
        case 0xB9:
            sub(reg.c, ALUFlag::DiscardResult);
            reg.pc++;

            return;

        // CP D
        case 0xBA:
            sub(reg.d, ALUFlag::DiscardResult);
            reg.pc++;

            return;

        // CP E
        case 0xBB:
            sub(reg.e, ALUFlag::DiscardResult);
            reg.pc++;

            return;

        // CP H
        case 0xBC:
            sub(reg.h, ALUFlag::DiscardResult);
            reg.pc++;

            return;

        // CP L
        case 0xBD:
            sub(reg.l, ALUFlag::DiscardResult);
            reg.pc++;

            return;

        // CP A
        case 0xBF:
            sub(reg.a, ALUFlag::DiscardResult);
            reg.pc++;

            return;

        // RET NZ
        case 0xC0:
            ret(!(reg.f & Flag::Zero));
            return;

        // POP BC
        case 0xC1:
            reg.c = m_bus.read(reg.sp++);
            reg.b = m_bus.read(reg.sp++);

            reg.pc++;
            return;

        // JP NZ, $imm16
        case 0xC2:
            jp(!(reg.f & Flag::Zero));
            return;

        // JP $imm16
        case 0xC3:
            jp(true);
            return;

        // CALL NZ, $imm16
        case 0xC4:
            call(!(reg.f & Flag::Zero));
            return;

        // PUSH BC
        case 0xC5:
            m_bus.write(--reg.sp, reg.b);
            m_bus.write(--reg.sp, reg.c);

            reg.pc++;
            return;

        // ADD A, $imm8
        case 0xC6:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            add(imm);

            reg.pc += 2;
            return;
        }

        // RST $00
        case 0xC7:
            rst(0x0000);
            return;

        // RET Z
        case 0xC8:
            ret(reg.f & Flag::Zero);
            return;

        // RET
        case 0xC9:
            ret(true);
            return;

        // JP Z, $imm16
        case 0xCA:
            jp(reg.f & Flag::Zero);
            return;

        // CB-prefixed instruction
        case 0xCB:
        {
            const uint8_t instruction{ m_bus.read(reg.pc + 1) };

            switch (instruction)
            {
                // RLC B
                case 0x00:
                    reg.b = rlc(reg.b);
                    reg.pc += 2;

                    return;

                // RLC C
                case 0x01:
                    reg.c = rlc(reg.c);
                    reg.pc += 2;

                    return;

                // RLC D
                case 0x02:
                    reg.d = rlc(reg.d);
                    reg.pc += 2;

                    return;

                // RLC E
                case 0x03:
                    reg.e = rlc(reg.e);
                    reg.pc += 2;

                    return;

                // RLC H
                case 0x04:
                    reg.h = rlc(reg.h);
                    reg.pc += 2;

                    return;

                // RLC L
                case 0x05:
                    reg.l = rlc(reg.l);
                    reg.pc += 2;

                    return;

                // RLC A
                case 0x07:
                    reg.a = rlc(reg.a);
                    reg.pc += 2;

                    return;

                // RRC B
                case 0x08:
                    reg.b = rrc(reg.b);
                    reg.pc += 2;

                    return;

                // RRC C
                case 0x09:
                    reg.c = rrc(reg.c);
                    reg.pc += 2;

                    return;

                // RRC D
                case 0x0A:
                    reg.d = rrc(reg.d);
                    reg.pc += 2;

                    return;

                // RRC E
                case 0x0B:
                    reg.e = rrc(reg.e);
                    reg.pc += 2;

                    return;

                // RRC H
                case 0x0C:
                    reg.h = rrc(reg.h);
                    reg.pc += 2;

                    return;

                // RRC L
                case 0x0D:
                    reg.l = rrc(reg.l);
                    reg.pc += 2;

                    return;

                // RRC A
                case 0x0F:
                    reg.a = rrc(reg.a);
                    reg.pc += 2;

                    return;

                // RL B
                case 0x10:
                    reg.b = rl(reg.b);
                    reg.pc += 2;

                    return;

                // RL C
                case 0x11:
                    reg.c = rl(reg.c);
                    reg.pc += 2;

                    return;

                // RL D
                case 0x12:
                    reg.d = rl(reg.d);
                    reg.pc += 2;

                    return;

                // RL E
                case 0x13:
                    reg.e = rl(reg.e);
                    reg.pc += 2;

                    return;

                // RL H
                case 0x14:
                    reg.h = rl(reg.h);
                    reg.pc += 2;

                    return;

                // RL L
                case 0x15:
                    reg.l = rl(reg.l);
                    reg.pc += 2;

                    return;

                // RL A
                case 0x17:
                    reg.a = rl(reg.a);
                    reg.pc += 2;

                    return;

                // RR B
                case 0x18:
                    reg.b = rr(reg.b);
                    reg.pc += 2;

                    return;

                // RR C
                case 0x19:
                    reg.c = rr(reg.c);
                    reg.pc += 2;

                    return;

                // RR D
                case 0x1A:
                    reg.d = rr(reg.d);
                    reg.pc += 2;

                    return;

                // RR E
                case 0x1B:
                    reg.e = rr(reg.e);
                    reg.pc += 2;

                    return;

                // RR H
                case 0x1C:
                    reg.h = rr(reg.h);
                    reg.pc += 2;

                    return;

                // RR L
                case 0x1D:
                    reg.l = rr(reg.l);
                    reg.pc += 2;

                    return;

                // RR A
                case 0x1F:
                    reg.a = rr(reg.a);
                    reg.pc += 2;

                    return;

                // SLA B
                case 0x20:
                    reg.b = sla(reg.b);
                    reg.pc += 2;

                    return;

                // SLA C
                case 0x21:
                    reg.c = sla(reg.c);
                    reg.pc += 2;

                    return;

                // SLA D
                case 0x22:
                    reg.d = sla(reg.d);
                    reg.pc += 2;

                    return;

                // SLA E
                case 0x23:
                    reg.e = sla(reg.e);
                    reg.pc += 2;

                    return;

                // SLA H
                case 0x24:
                    reg.h = sla(reg.h);
                    reg.pc += 2;

                    return;

                // SLA L
                case 0x25:
                    reg.l = sla(reg.l);
                    reg.pc += 2;

                    return;

                // SLA A
                case 0x27:
                    reg.a = sla(reg.a);
                    reg.pc += 2;

                    return;

                // SRA B
                case 0x28:
                    reg.b = sra(reg.b);
                    reg.pc += 2;

                    return;

                // SRA C
                case 0x29:
                    reg.c = sra(reg.c);
                    reg.pc += 2;

                    return;

                // SRA D
                case 0x2A:
                    reg.d = sra(reg.d);
                    reg.pc += 2;

                    return;

                // SRA E
                case 0x2B:
                    reg.e = sra(reg.e);
                    reg.pc += 2;

                    return;

                // SRA H
                case 0x2C:
                    reg.h = sra(reg.h);
                    reg.pc += 2;

                    return;

                // SRA L
                case 0x2D:
                    reg.l = sra(reg.l);
                    reg.pc += 2;

                    return;

                // SRA A
                case 0x2F:
                    reg.a = sra(reg.a);
                    reg.pc += 2;

                    return;

                // SWAP B
                case 0x30:
                    reg.b = ((reg.b & 0x0F) << 4) | (reg.b >> 4);
                    reg.f = (reg.b == 0) ? 0x80 : 0x00;

                    reg.pc += 2;
                    return;

                // SWAP C
                case 0x31:
                    reg.c = ((reg.c & 0x0F) << 4) | (reg.c >> 4);
                    reg.f = (reg.c == 0) ? 0x80 : 0x00;

                    reg.pc += 2;
                    return;

                // SWAP D
                case 0x32:
                    reg.d = ((reg.d & 0x0F) << 4) | (reg.d >> 4);
                    reg.f = (reg.d == 0) ? 0x80 : 0x00;

                    reg.pc += 2;
                    return;

                // SWAP E
                case 0x33:
                    reg.e = ((reg.e & 0x0F) << 4) | (reg.e >> 4);
                    reg.f = (reg.e == 0) ? 0x80 : 0x00;

                    reg.pc += 2;
                    return;

                // SWAP H
                case 0x34:
                    reg.h = ((reg.h & 0x0F) << 4) | (reg.h >> 4);
                    reg.f = (reg.h == 0) ? 0x80 : 0x00;

                    reg.pc += 2;
                    return;

                // SWAP L
                case 0x35:
                    reg.l = ((reg.l & 0x0F) << 4) | (reg.l >> 4);
                    reg.f = (reg.l == 0) ? 0x80 : 0x00;

                    reg.pc += 2;
                    return;

                // SWAP A
                case 0x37:
                    reg.a = ((reg.a & 0x0F) << 4) | (reg.a >> 4);
                    reg.f = (reg.a == 0) ? 0x80 : 0x00;

                    reg.pc += 2;
                    return;

                // SRL B
                case 0x38:
                    reg.b = srl(reg.b);
                    reg.pc += 2;

                    return;

                // SRL C
                case 0x39:
                    reg.c = srl(reg.c);
                    reg.pc += 2;

                    return;

                // SRL D
                case 0x3A:
                    reg.d = srl(reg.d);
                    reg.pc += 2;

                    return;

                // SRL E
                case 0x3B:
                    reg.e = srl(reg.e);
                    reg.pc += 2;

                    return;

                // SRL H
                case 0x3C:
                    reg.h = srl(reg.h);
                    reg.pc += 2;

                    return;

                // SRL L
                case 0x3D:
                    reg.l = srl(reg.l);
                    reg.pc += 2;

                    return;

                // SRL A
                case 0x3F:
                    reg.a = srl(reg.a);
                    reg.pc += 2;

                    return;

                // BIT 0, B
                case 0x40:
                    bit(0, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 0, C
                case 0x41:
                    bit(0, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 0, D
                case 0x42:
                    bit(0, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 0, E
                case 0x43:
                    bit(0, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 0, H
                case 0x44:
                    bit(0, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 0, L
                case 0x45:
                    bit(0, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 0, A
                case 0x47:
                    bit(0, reg.a);
                    reg.pc += 2;

                    return;

                // BIT 1, B
                case 0x48:
                    bit(1, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 1, C
                case 0x49:
                    bit(1, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 1, D
                case 0x4A:
                    bit(1, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 1, E
                case 0x4B:
                    bit(1, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 1, H
                case 0x4C:
                    bit(1, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 1, L
                case 0x4D:
                    bit(1, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 1, A
                case 0x4F:
                    bit(1, reg.a);
                    reg.pc += 2;

                    return;

                // BIT 2, B
                case 0x50:
                    bit(2, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 2, C
                case 0x51:
                    bit(2, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 2, D
                case 0x52:
                    bit(2, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 2, E
                case 0x53:
                    bit(2, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 2, H
                case 0x54:
                    bit(2, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 2, L
                case 0x55:
                    bit(2, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 2, A
                case 0x57:
                    bit(2, reg.a);
                    reg.pc += 2;

                    return;

                // BIT 3, B
                case 0x58:
                    bit(3, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 3, C
                case 0x59:
                    bit(3, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 3, D
                case 0x5A:
                    bit(3, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 3, E
                case 0x5B:
                    bit(3, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 3, H
                case 0x5C:
                    bit(3, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 3, L
                case 0x5D:
                    bit(3, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 3, A
                case 0x5F:
                    bit(3, reg.a);
                    reg.pc += 2;

                    return;

                // BIT 4, B
                case 0x60:
                    bit(4, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 4, C
                case 0x61:
                    bit(4, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 4, D
                case 0x62:
                    bit(4, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 4, E
                case 0x63:
                    bit(4, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 4, H
                case 0x64:
                    bit(4, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 4, L
                case 0x65:
                    bit(4, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 4, A
                case 0x67:
                    bit(4, reg.a);
                    reg.pc += 2;

                    return;

                // BIT 5, B
                case 0x68:
                    bit(5, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 5, C
                case 0x69:
                    bit(5, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 5, D
                case 0x6A:
                    bit(5, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 5, E
                case 0x6B:
                    bit(5, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 5, H
                case 0x6C:
                    bit(5, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 5, L
                case 0x6D:
                    bit(5, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 5, A
                case 0x6F:
                    bit(5, reg.a);
                    reg.pc += 2;

                    return;

                // BIT 6, B
                case 0x70:
                    bit(6, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 6, C
                case 0x71:
                    bit(6, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 6, D
                case 0x72:
                    bit(6, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 6, E
                case 0x73:
                    bit(6, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 6, H
                case 0x74:
                    bit(6, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 6, L
                case 0x75:
                    bit(6, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 6, A
                case 0x77:
                    bit(6, reg.a);
                    reg.pc += 2;

                    return;

                // BIT 7, B
                case 0x78:
                    bit(7, reg.b);
                    reg.pc += 2;

                    return;

                // BIT 7, C
                case 0x79:
                    bit(7, reg.c);
                    reg.pc += 2;

                    return;

                // BIT 7, D
                case 0x7A:
                    bit(7, reg.d);
                    reg.pc += 2;

                    return;

                // BIT 7, E
                case 0x7B:
                    bit(7, reg.e);
                    reg.pc += 2;

                    return;

                // BIT 7, H
                case 0x7C:
                    bit(7, reg.h);
                    reg.pc += 2;

                    return;

                // BIT 7, L
                case 0x7D:
                    bit(7, reg.l);
                    reg.pc += 2;

                    return;

                // BIT 7, A
                case 0x7F:
                    bit(7, reg.a);
                    reg.pc += 2;

                    return;

                // RES 0, B
                case 0x80:
                    reg.b &= ~(1 << 0);
                    reg.pc += 2;

                    return;

                // RES 0, C
                case 0x81:
                    reg.c &= ~(1 << 0);
                    reg.pc += 2;

                    return;

                // RES 0, D
                case 0x82:
                    reg.d &= ~(1 << 0);
                    reg.pc += 2;

                    return;

                // RES 0, E
                case 0x83:
                    reg.e &= ~(1 << 0);
                    reg.pc += 2;

                    return;

                // RES 0, H
                case 0x84:
                    reg.h &= ~(1 << 0);
                    reg.pc += 2;

                    return;

                // RES 0, L
                case 0x85:
                    reg.l &= ~(1 << 0);
                    reg.pc += 2;

                    return;

                // RES 0, A
                case 0x87:
                    reg.a &= ~(1 << 0);
                    reg.pc += 2;

                    return;

                // RES 1, B
                case 0x88:
                    reg.b &= ~(1 << 1);
                    reg.pc += 2;

                    return;

                // RES 1, C
                case 0x89:
                    reg.c &= ~(1 << 1);
                    reg.pc += 2;

                    return;

                // RES 1, D
                case 0x8A:
                    reg.d &= ~(1 << 1);
                    reg.pc += 2;

                    return;

                // RES 1, E
                case 0x8B:
                    reg.e &= ~(1 << 1);
                    reg.pc += 2;

                    return;

                // RES 1, H
                case 0x8C:
                    reg.h &= ~(1 << 1);
                    reg.pc += 2;

                    return;

                // RES 1, L
                case 0x8D:
                    reg.l &= ~(1 << 1);
                    reg.pc += 2;

                    return;

                // RES 1, A
                case 0x8F:
                    reg.a &= ~(1 << 1);
                    reg.pc += 2;

                    return;

                // RES 2, B
                case 0x90:
                    reg.b &= ~(1 << 2);
                    reg.pc += 2;

                    return;

                // RES 2, C
                case 0x91:
                    reg.c &= ~(1 << 2);
                    reg.pc += 2;

                    return;

                // RES 2, D
                case 0x92:
                    reg.d &= ~(1 << 2);
                    reg.pc += 2;

                    return;

                // RES 2, E
                case 0x93:
                    reg.e &= ~(1 << 2);
                    reg.pc += 2;

                    return;

                // RES 2, H
                case 0x94:
                    reg.h &= ~(1 << 2);
                    reg.pc += 2;

                    return;

                // RES 2, L
                case 0x95:
                    reg.l &= ~(1 << 2);
                    reg.pc += 2;

                    return;

                // RES 2, A
                case 0x97:
                    reg.a &= ~(1 << 2);
                    reg.pc += 2;

                    return;

                // RES 3, B
                case 0x98:
                    reg.b &= ~(1 << 3);
                    reg.pc += 2;

                    return;

                // RES 3, C
                case 0x99:
                    reg.c &= ~(1 << 3);
                    reg.pc += 2;

                    return;

                // RES 3, D
                case 0x9A:
                    reg.d &= ~(1 << 3);
                    reg.pc += 2;

                    return;

                // RES 3, E
                case 0x9B:
                    reg.e &= ~(1 << 3);
                    reg.pc += 2;

                    return;

                // RES 3, H
                case 0x9C:
                    reg.h &= ~(1 << 3);
                    reg.pc += 2;

                    return;

                // RES 3, L
                case 0x9D:
                    reg.l &= ~(1 << 3);
                    reg.pc += 2;

                    return;

                // RES 3, A
                case 0x9F:
                    reg.a &= ~(1 << 3);
                    reg.pc += 2;

                    return;

                // RES 4, B
                case 0xA0:
                    reg.b &= ~(1 << 4);
                    reg.pc += 2;

                    return;

                // RES 4, C
                case 0xA1:
                    reg.c &= ~(1 << 4);
                    reg.pc += 2;

                    return;

                // RES 4, D
                case 0xA2:
                    reg.d &= ~(1 << 4);
                    reg.pc += 2;

                    return;

                // RES 4, E
                case 0xA3:
                    reg.e &= ~(1 << 4);
                    reg.pc += 2;

                    return;

                // RES 4, H
                case 0xA4:
                    reg.h &= ~(1 << 4);
                    reg.pc += 2;

                    return;

                // RES 4, L
                case 0xA5:
                    reg.l &= ~(1 << 4);
                    reg.pc += 2;

                    return;

                // RES 4, A
                case 0xA7:
                    reg.a &= ~(1 << 4);
                    reg.pc += 2;

                    return;

                // RES 5, B
                case 0xA8:
                    reg.b &= ~(1 << 5);
                    reg.pc += 2;

                    return;

                // RES 5, C
                case 0xA9:
                    reg.c &= ~(1 << 5);
                    reg.pc += 2;

                    return;

                // RES 5, D
                case 0xAA:
                    reg.d &= ~(1 << 5);
                    reg.pc += 2;

                    return;

                // RES 5, E
                case 0xAB:
                    reg.e &= ~(1 << 5);
                    reg.pc += 2;

                    return;

                // RES 5, H
                case 0xAC:
                    reg.h &= ~(1 << 5);
                    reg.pc += 2;

                    return;

                // RES 5, L
                case 0xAD:
                    reg.l &= ~(1 << 5);
                    reg.pc += 2;

                    return;

                // RES 5, A
                case 0xAF:
                    reg.a &= ~(1 << 5);
                    reg.pc += 2;

                    return;

                // RES 6, B
                case 0xB0:
                    reg.b &= ~(1 << 6);
                    reg.pc += 2;

                    return;

                // RES 6, C
                case 0xB1:
                    reg.c &= ~(1 << 6);
                    reg.pc += 2;

                    return;

                // RES 6, D
                case 0xB2:
                    reg.d &= ~(1 << 6);
                    reg.pc += 2;

                    return;

                // RES 6, E
                case 0xB3:
                    reg.e &= ~(1 << 6);
                    reg.pc += 2;

                    return;

                // RES 6, H
                case 0xB4:
                    reg.h &= ~(1 << 6);
                    reg.pc += 2;

                    return;

                // RES 6, L
                case 0xB5:
                    reg.l &= ~(1 << 6);
                    reg.pc += 2;

                    return;

                // RES 6, A
                case 0xB7:
                    reg.a &= ~(1 << 6);
                    reg.pc += 2;

                    return;

                // RES 7, B
                case 0xB8:
                    reg.b &= ~(1 << 7);
                    reg.pc += 2;

                    return;

                // RES 7, C
                case 0xB9:
                    reg.c &= ~(1 << 7);
                    reg.pc += 2;

                    return;

                // RES 7, D
                case 0xBA:
                    reg.d &= ~(1 << 7);
                    reg.pc += 2;

                    return;

                // RES 7, E
                case 0xBB:
                    reg.e &= ~(1 << 7);
                    reg.pc += 2;

                    return;

                // RES 7, H
                case 0xBC:
                    reg.h &= ~(1 << 7);
                    reg.pc += 2;

                    return;

                // RES 7, L
                case 0xBD:
                    reg.l &= ~(1 << 7);
                    reg.pc += 2;

                    return;

                // RES 7, A
                case 0xBF:
                    reg.a &= ~(1 << 7);
                    reg.pc += 2;

                    return;

                // SET 0, B
                case 0xC0:
                    reg.b |= (1 << 0);
                    reg.pc += 2;

                    return;

                // SET 0, C
                case 0xC1:
                    reg.c |= (1 << 0);
                    reg.pc += 2;

                    return;

                // SET 0, D
                case 0xC2:
                    reg.d |= (1 << 0);
                    reg.pc += 2;

                    return;

                // SET 0, E
                case 0xC3:
                    reg.e |= (1 << 0);
                    reg.pc += 2;

                    return;

                // SET 0, H
                case 0xC4:
                    reg.h |= (1 << 0);
                    reg.pc += 2;

                    return;

                // SET 0, L
                case 0xC5:
                    reg.l |= (1 << 0);
                    reg.pc += 2;

                    return;

                // SET 0, A
                case 0xC7:
                    reg.a |= (1 << 0);
                    reg.pc += 2;

                    return;

                // SET 1, B
                case 0xC8:
                    reg.b |= (1 << 1);
                    reg.pc += 2;

                    return;

                // SET 1, C
                case 0xC9:
                    reg.c |= (1 << 1);
                    reg.pc += 2;

                    return;

                // SET 1, D
                case 0xCA:
                    reg.d |= (1 << 1);
                    reg.pc += 2;

                    return;

                // SET 1, E
                case 0xCB:
                    reg.e |= (1 << 1);
                    reg.pc += 2;

                    return;

                // SET 1, H
                case 0xCC:
                    reg.h |= (1 << 1);
                    reg.pc += 2;

                    return;

                // SET 1, L
                case 0xCD:
                    reg.l |= (1 << 1);
                    reg.pc += 2;

                    return;

                // SET 1, A
                case 0xCF:
                    reg.a |= (1 << 1);
                    reg.pc += 2;

                    return;

                // SET 2, B
                case 0xD0:
                    reg.b |= (1 << 2);
                    reg.pc += 2;

                    return;

                // SET 2, C
                case 0xD1:
                    reg.c |= (1 << 2);
                    reg.pc += 2;

                    return;

                // SET 2, D
                case 0xD2:
                    reg.d |= (1 << 2);
                    reg.pc += 2;

                    return;

                // SET 2, E
                case 0xD3:
                    reg.e |= (1 << 2);
                    reg.pc += 2;

                    return;

                // SET 2, H
                case 0xD4:
                    reg.h |= (1 << 2);
                    reg.pc += 2;

                    return;

                // SET 2, L
                case 0xD5:
                    reg.l |= (1 << 2);
                    reg.pc += 2;

                    return;

                // SET 2, A
                case 0xD7:
                    reg.a |= (1 << 2);
                    reg.pc += 2;

                    return;

                // SET 3, B
                case 0xD8:
                    reg.b |= (1 << 3);
                    reg.pc += 2;

                    return;

                // SET 3, C
                case 0xD9:
                    reg.c |= (1 << 3);
                    reg.pc += 2;

                    return;

                // SET 3, D
                case 0xDA:
                    reg.d |= (1 << 3);
                    reg.pc += 2;

                    return;

                // SET 3, E
                case 0xDB:
                    reg.e |= (1 << 3);
                    reg.pc += 2;

                    return;

                // SET 3, H
                case 0xDC:
                    reg.h |= (1 << 3);
                    reg.pc += 2;

                    return;

                // SET 3, L
                case 0xDD:
                    reg.l |= (1 << 3);
                    reg.pc += 2;

                    return;

                // SET 3, A
                case 0xDF:
                    reg.a |= (1 << 3);
                    reg.pc += 2;

                    return;

                // SET 4, B
                case 0xE0:
                    reg.b |= (1 << 4);
                    reg.pc += 2;

                    return;

                // SET 4, C
                case 0xE1:
                    reg.c |= (1 << 4);
                    reg.pc += 2;

                    return;

                // SET 4, D
                case 0xE2:
                    reg.d |= (1 << 4);
                    reg.pc += 2;

                    return;

                // SET 4, E
                case 0xE3:
                    reg.e |= (1 << 4);
                    reg.pc += 2;

                    return;

                // SET 4, H
                case 0xE4:
                    reg.h |= (1 << 4);
                    reg.pc += 2;

                    return;

                // SET 4, L
                case 0xE5:
                    reg.l |= (1 << 4);
                    reg.pc += 2;

                    return;

                // SET 4, A
                case 0xE7:
                    reg.a |= (1 << 4);
                    reg.pc += 2;

                    return;

                // SET 5, B
                case 0xE8:
                    reg.b |= (1 << 5);
                    reg.pc += 2;

                    return;

                // SET 5, C
                case 0xE9:
                    reg.c |= (1 << 5);
                    reg.pc += 2;

                    return;

                // SET 5, D
                case 0xEA:
                    reg.d |= (1 << 5);
                    reg.pc += 2;

                    return;

                // SET 5, E
                case 0xEB:
                    reg.e |= (1 << 5);
                    reg.pc += 2;

                    return;

                // SET 5, H
                case 0xEC:
                    reg.h |= (1 << 5);
                    reg.pc += 2;

                    return;

                // SET 5, L
                case 0xED:
                    reg.l |= (1 << 5);
                    reg.pc += 2;

                    return;

                // SET 5, A
                case 0xEF:
                    reg.a |= (1 << 5);
                    reg.pc += 2;

                    return;

                // SET 6, B
                case 0xF0:
                    reg.b |= (1 << 6);
                    reg.pc += 2;

                    return;

                // SET 6, C
                case 0xF1:
                    reg.c |= (1 << 6);
                    reg.pc += 2;

                    return;

                // SET 6, D
                case 0xF2:
                    reg.d |= (1 << 6);
                    reg.pc += 2;

                    return;

                // SET 6, E
                case 0xF3:
                    reg.e |= (1 << 6);
                    reg.pc += 2;

                    return;

                // SET 6, H
                case 0xF4:
                    reg.h |= (1 << 6);
                    reg.pc += 2;

                    return;

                // SET 6, L
                case 0xF5:
                    reg.l |= (1 << 6);
                    reg.pc += 2;

                    return;

                // SET 6, A
                case 0xF7:
                    reg.a |= (1 << 6);
                    reg.pc += 2;

                    return;

                // SET 7, B
                case 0xF8:
                    reg.b |= (1 << 7);
                    reg.pc += 2;

                    return;

                // SET 7, C
                case 0xF9:
                    reg.c |= (1 << 7);
                    reg.pc += 2;

                    return;

                // SET 7, D
                case 0xFA:
                    reg.d |= (1 << 7);
                    reg.pc += 2;

                    return;

                // SET 7, E
                case 0xFB:
                    reg.e |= (1 << 7);
                    reg.pc += 2;

                    return;

                // SET 7, H
                case 0xFC:
                    reg.h |= (1 << 7);
                    reg.pc += 2;

                    return;

                // SET 7, L
                case 0xFD:
                    reg.l |= (1 << 7);
                    reg.pc += 2;

                    return;

                // SET 7, A
                case 0xFF:
                    reg.a |= (1 << 7);
                    reg.pc += 2;

                    return;

                default:
                    __debugbreak();
                    return;
            }
        }

        // CALL Z, $imm16
        case 0xCC:
            call(reg.f & Flag::Zero);
            return;

        // CALL $imm16
        case 0xCD:
            call(true);
            return;

        // ADC A, $imm8
        case 0xCE:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            add(imm, ALUFlag::WithCarry);
            reg.pc += 2;

            return;
        }

        // RST $08
        case 0xCF:
            rst(0x0008);
            return;

        // RET NC
        case 0xD0:
            ret(!(reg.f & Flag::Carry));
            return;

        // POP DE
        case 0xD1:
            reg.e = m_bus.read(reg.sp++);
            reg.d = m_bus.read(reg.sp++);

            reg.pc++;
            return;

        // JP NC, $imm16
        case 0xD2:
            jp(!(reg.f & Flag::Carry));
            return;

        // CALL NC, $imm16
        case 0xD4:
            call(!(reg.f & Flag::Carry));
            return;

        // PUSH DE
        case 0xD5:
            m_bus.write(--reg.sp, reg.d);
            m_bus.write(--reg.sp, reg.e);

            reg.pc++;
            return;

        // SUB $imm8
        case 0xD6:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            sub(imm);
            reg.pc += 2;

            return;
        }

        // RST $10
        case 0xD7:
            rst(0x0010);
            return;

        // RET C
        case 0xD8:
            ret(reg.f & Flag::Carry);
            return;

        // RETI
        case 0xD9:
            ret(true);
            return;

        // JP C, $imm16
        case 0xDA:
            jp(reg.f & Flag::Carry);
            return;

        // CALL C, $imm16
        case 0xDC:
            call(reg.f & Flag::Carry);
            return;

        // SBC A, $imm8
        case 0xDE:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };
            sub(imm, ALUFlag::WithCarry);

            reg.pc += 2;
            return;
        }

        // RST $18
        case 0xDF:
            rst(0x0018);
            return;

        // LDH ($imm8), A
        case 0xE0:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            m_bus.write(0xFF00 + imm, reg.a);
            reg.pc += 2;

            return;
        }

        // POP HL
        case 0xE1:
            reg.l = m_bus.read(reg.sp++);
            reg.h = m_bus.read(reg.sp++);

            reg.pc++;
            return;

        // LD (C), A
        case 0xE2:
            m_bus.write(0xFF00 + reg.c, reg.a);
            reg.pc++;

            return;

        // PUSH HL
        case 0xE5:
            m_bus.write(--reg.sp, reg.h);
            m_bus.write(--reg.sp, reg.l);

            reg.pc++;
            return;

        // AND $imm8
        case 0xE6:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            reg.a &= imm;
            reg.f = (reg.a == 0) ? 0xA0 : 0x20;

            reg.pc += 2;
            return;
        }

        // RST $20
        case 0xE7:
            rst(0x0020);
            return;

        // ADD SP, $simm8
        case 0xE8:
        {
            reg.f &= ~Flag::Zero;
            reg.f &= ~Flag::Subtract;

            const int8_t imm{ static_cast<int8_t>(m_bus.read(reg.pc + 1)) };

            const int sum = reg.sp + imm;

            if ((reg.sp ^ imm ^ sum) & 0x10)
            {
                reg.f |= Flag::HalfCarry;
            }
            else
            {
                reg.f &= ~Flag::HalfCarry;
            }

            if ((reg.sp ^ imm ^ sum) & 0x100)
            {
                reg.f |= Flag::Carry;
            }
            else
            {
                reg.f &= ~Flag::Carry;
            }

            reg.sp = static_cast<uint16_t>(sum);
            reg.pc += 2;

            return;
        }

        // JP (HL)
        case 0xE9:
            reg.pc = hl();
            return;

        // LD ($imm16), A
        case 0xEA:
        {
            const uint8_t lo{ m_bus.read(reg.pc + 1) };
            const uint8_t hi{ m_bus.read(reg.pc + 2) };

            const uint16_t address = (hi << 8) | lo;

            m_bus.write(address, reg.a);
            reg.pc += 3;

            return;
        }

        // XOR $imm8
        case 0xEE:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            reg.a ^= imm;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc += 2;
            return;
        }

        // RST $28
        case 0xEF:
            rst(0x0028);
            return;

        // LDH A, ($imm8)
        case 0xF0:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            reg.a = m_bus.read(0xFF00 + imm);
            reg.pc += 2;

            return;
        }

        // POP AF
        case 0xF1:
            reg.f = m_bus.read(reg.sp++) & ~0x0F;
            reg.a = m_bus.read(reg.sp++);

            reg.pc++;
            return;

        // LD A, (C)
        case 0xF2:
            reg.a = m_bus.read(0xFF00 + reg.c);
            reg.pc++;

            return;

        // DI
        case 0xF3:
            reg.pc++;
            return;

        // PUSH AF
        case 0xF5:
            m_bus.write(--reg.sp, reg.a);
            m_bus.write(--reg.sp, reg.f);

            reg.pc++;
            return;

        // OR $imm8
        case 0xF6:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            reg.a |= imm;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc += 2;
            return;
        }

        // RST $30
        case 0xF7:
            rst(0x0030);
            return;

        // LD HL, SP+r8
        case 0xF8:
        {
            reg.f &= ~Flag::Zero;
            reg.f &= ~Flag::Subtract;

            const int8_t imm{ static_cast<int8_t>(m_bus.read(reg.pc + 1)) };

            const int sum = reg.sp + imm;

            if ((reg.sp ^ imm ^ sum) & 0x10)
            {
                reg.f |= Flag::HalfCarry;
            }
            else
            {
                reg.f &= ~Flag::HalfCarry;
            }

            if ((reg.sp ^ imm ^ sum) & 0x100)
            {
                reg.f |= Flag::Carry;
            }
            else
            {
                reg.f &= ~Flag::Carry;
            }

            const uint16_t final{ static_cast<uint16_t>(sum) };

            reg.h = final >> 8;
            reg.l = final & 0x00FF;

            reg.pc += 2;
            return;
        }

        // LD SP, HL
        case 0xF9:
            reg.sp = hl();
            reg.pc++;

            return;

        // LD A, ($imm16)
        case 0xFA:
        {
            const uint8_t lo{ m_bus.read(reg.pc + 1) };
            const uint8_t hi{ m_bus.read(reg.pc + 2) };

            const uint16_t address = (hi << 8) | lo;

            reg.a = m_bus.read(address);
            reg.pc += 3;

            return;
        }

        // CP $imm8
        case 0xFE:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            sub(imm, ALUFlag::DiscardResult);
            reg.pc += 2;

            return;
        }

        // RST $38
        case 0xFF:
            rst(0x0038);
            return;

        default:
            __debugbreak();
    }
}