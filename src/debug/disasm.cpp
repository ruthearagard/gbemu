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

// Required for `fmt::sprintf()`.
#include <fmt/printf.h>

// Required for the `Disassembler` class.
#include "disasm.h"

// Required for the `GameBoy::SystemBus` class.
#include "../libgbemu/include/bus.h"

// Required for the `GameBoy::CPU` class.
#include "../libgbemu/include/cpu.h"

Disassembler::Disassembler(const GameBoy::SystemBus& bus,
                           const GameBoy::CPU& cpu) noexcept : m_bus(bus),
                                                               m_cpu(cpu)
{ }

// Disassembles the current instruction before execution.
auto Disassembler::before() noexcept -> void
{
    disasm = fmt::sprintf("$%04X: ", m_cpu.reg.pc);

    auto instruction{ opcodes[m_bus.read(m_cpu.reg.pc)] };

    if (instruction.empty())
    {
        instruction = "illegal";
    }
    else if (instruction == "CB_INSTR")
    {
        instruction = cb_opcodes[m_bus.read(m_cpu.reg.pc + 1)];
    }

    // Go through each character, one at a time.
    for (auto index{ 0 }; index < instruction.length();)
    {
        // If the character is not our interpolation token...
        if (instruction[index] != '$')
        {
            // ...then it can be safely added to our result.
            disasm += instruction[index++];
            continue;
        }

        if (instruction.compare(index, 5, "$imm8") == 0)
        {
            const uint8_t imm{ m_bus.read(m_cpu.reg.pc + 1) };

            disasm += fmt::sprintf("$%02X", imm);
            index  += 5;
        }

        if (instruction.compare(index, 6, "$simm8") == 0)
        {
            const int8_t imm
            {
                static_cast<int8_t>(m_bus.read(m_cpu.reg.pc + 1))
            };

            disasm += fmt::sprintf("%s$%02X", (imm < 0) ? "-" : "", abs(imm));
            index  += 6;
        }

        if (instruction.compare(index, 6, "$imm16") == 0)
        {
            const uint8_t lo{ m_bus.read(m_cpu.reg.pc + 1) };
            const uint8_t hi{ m_bus.read(m_cpu.reg.pc + 2) };

            const uint16_t imm = (hi << 8) | lo;

            disasm += fmt::sprintf("$%04X", imm);
            index  += 6;
        }

        if (instruction.compare(index, 7, "$branch") == 0)
        {
            const int8_t imm
            {
                static_cast<int8_t>(m_bus.read(m_cpu.reg.pc + 1))
            };

            const uint16_t address = imm + m_cpu.reg.pc + 2;

            disasm += fmt::sprintf("$%04X", address);
            index  += 7;
        }
    }
}

// Disassembles the current instruction after execution.
auto Disassembler::after() noexcept -> std::string
{
    while (disasm.size() < 30)
    {
        disasm += " ";
    }

    disasm += fmt::sprintf("; [BC=0x%04X, DE=0x%04X, HL=0x%04X, AF=0x%04X, "
                           "SP=0x%04X]",
                           m_cpu.reg.bc,
                           m_cpu.reg.de,
                           m_cpu.reg.hl,
                           m_cpu.reg.af,
                           m_cpu.reg.sp);
    return disasm;
}