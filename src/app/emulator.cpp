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

#include <fstream>
#include <stdexcept>
#include <qapplication.h>
#include <qthread.h>
#include "emulator.h"

/// @brief Initializes the Game Boy system execution interface.
Emulator::Emulator() noexcept : disasm(bus, cpu)
{ }

/// @brief Starts the run loop. Does nothing if the loop is already running.
auto Emulator::start_run_loop() noexcept -> void
{
    if (!running)
    {
        running = true;
        start();
    }
}

/// @brief Pauses the run loop, preserving the current state. Does nothing if
/// the emulator is not running.
auto Emulator::pause_run_loop() noexcept -> void
{
    if (running)
    {
        running = false;
        quit();
    }
}

/// @brief Generates a cartridge and sets it up to be executed.
/// @param The data to use to generate the cartridge.
auto Emulator::cartridge(const std::vector<uint8_t>& data) -> void
{
    // We want to stop the run loop if it is running, since the current
    // `m_cart` will be going away if this is successful.
    const auto was_running{ running };
    pause_run_loop();

    try
    {
        m_cart = cart(data);
        reset();
        start_run_loop();
    }
    catch (std::runtime_error& err)
    {
        // `m_cart` unchanged, so it's okay to continue with the current
        // execution if there was one prior.
        if (was_running)
        {
            start_run_loop();
        }
        throw;
    }
}

/// @brief The starting point for the thread.
auto Emulator::run() -> void
{
    std::ofstream trace_file{ "trace.txt" };

    while (running)
    {
        const auto start{ std::chrono::steady_clock::now() };
            while (cycles < max_cycles)
            {
                disasm.before();
                cycles += step();
                trace_file << disasm.after() << std::endl;
            }

            cycles -= max_cycles;
            emit render_frame(bus.ppu.screen_data);
        const auto end{ std::chrono::steady_clock::now() };

        const auto diff
        {
            std::chrono::duration_cast<std::chrono::milliseconds>
            (end - start).count()
        };

        if (diff < (1000 / 60))
        {
            QThread::msleep((1000 / 60) - diff);
        }
    }
}
