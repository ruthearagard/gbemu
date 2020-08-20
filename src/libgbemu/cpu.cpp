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

CPU::CPU(SystemBus& bus) : m_bus(bus)
{
    reset();
}

// Handles the `INC r` instruction.
auto CPU::inc(uint8_t r) noexcept -> uint8_t
{
    reg.f &= ~Flag::Subtract;

    r++;

    if ((r & 0x0F) == 0x0F)
    {
        reg.f |= Flag::HalfCarry;
    }
    else
    {
        reg.f &= ~Flag::HalfCarry;
    }

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

    bool discard_result{ false };
    uint8_t diff = reg.a - subtrahend;

    if (flag == ALUFlag::WithCarry)
    {
        const bool carry{ (reg.f & Flag::Carry) != 0 };
        diff -= carry;
    }
    else if (flag == ALUFlag::DiscardResult)
    {
        discard_result = true;
    }

    if (diff == 0)
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

    if (reg.a < subtrahend)
    {
        reg.f |= Flag::Carry;
    }
    else
    {
        reg.f &= ~Flag::Carry;
    }

    if (!discard_result)
    {
        reg.a = diff;
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

        // JR $branch
        case 0x18:
            jr(true);
            return;

        // LD A, (DE)
        case 0x1A:
            reg.a = m_bus.read(de());
            reg.pc++;

            return;

        // INC E
        case 0x1C:
            reg.e = inc(reg.e);
            reg.pc++;

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

        // JR Z, $branch
        case 0x28:
            jr(reg.f & Flag::Zero);
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

        // LD A, $imm8
        case 0x3E:
            reg.a = m_bus.read(reg.pc + 1);
            reg.pc += 2;

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

        // LD E, A
        case 0x5F:
            reg.e = reg.a;
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

        // XOR C
        case 0xA9:
            reg.a ^= reg.c;
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

        // OR C
        case 0xB1:
            reg.a |= reg.c;
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // OR A
        case 0xB7:
            reg.f = (reg.a == 0) ? 0x80 : 0x00;

            reg.pc++;
            return;

        // POP BC
        case 0xC1:
            reg.c = m_bus.read(reg.sp++);
            reg.b = m_bus.read(reg.sp++);

            reg.pc++;
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

        // RET Z
        case 0xC8:
            ret(reg.f & Flag::Zero);
            return;

        // RET
        case 0xC9:
            ret(true);
            return;

        // CB-prefixed instruction
        case 0xCB:
        {
            const uint8_t instruction{ m_bus.read(reg.pc + 1) };

            switch (instruction)
            {
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

                // SRL B
                case 0x38:
                    reg.b = srl(reg.b);
                    reg.pc += 2;

                    return;

                default:
                    __debugbreak();
                    return;
            }
        }

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
            reg.f = m_bus.read(reg.sp++);
            reg.a = m_bus.read(reg.sp++);

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

        default:
            __debugbreak();
    }
}