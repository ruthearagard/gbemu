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

        // INC D
        case 0x14:
            reg.d = inc(reg.d);
            reg.pc++;

            return;

        // JR $branch
        case 0x18:
            jr(true);
            return;

        // INC E
        case 0x1C:
            reg.e = inc(reg.e);
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

        // LD SP, $imm16
        case 0x31:
        {
            const uint8_t lo{ m_bus.read(reg.pc + 1) };
            const uint8_t hi{ m_bus.read(reg.pc + 2) };

            reg.sp = (hi << 8) | lo;
            reg.pc += 3;

            return;
        }

        // LD A, $imm8
        case 0x3E:
            reg.a = m_bus.read(reg.pc + 1);
            reg.pc += 2;

            return;

        // LD B, A
        case 0x47:
            reg.b = reg.a;
            reg.pc++;

            return;

        // LD A, B
        case 0x78:
            reg.a = reg.b;
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

        // JP $imm16
        case 0xC3:
            jp(true);
            return;

        // RET
        case 0xC9:
            ret(true);
            return;

        // CALL $imm16
        case 0xCD:
            call(true);
            return;

        // LDH ($imm8), A
        case 0xE0:
        {
            const uint8_t imm{ m_bus.read(reg.pc + 1) };

            m_bus.write(0xFF00 + imm, reg.a);
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

        // DI
        case 0xF3:
            reg.pc++;
            return;

        default:
            __debugbreak();
    }
}