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

// Required for C++ exceptions.
#include <stdexcept>

// Required for the `QApplication` class.
#include <qapplication.h>

// Required for the `QThread` class.
#include <qthread.h>

// Required for the `Emulator` class.
#include "emulator.h"

Emulator::Emulator() noexcept
{ }

auto Emulator::start_run_loop() noexcept -> void
{
    if (!running)
    {
        running = true;
        start();
    }
}

auto Emulator::run() noexcept -> void
{
    cycles = 0;

    if (!running)
    {
        running = true;
        cycles = 0;
    }

    while (running)
    {
        const auto start{ std::chrono::steady_clock::now() };
            while (cycles < max_cycles)
            {
                cycles += step();
            }

            QApplication::processEvents(QEventLoop::AllEvents);
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

auto Emulator::pause() noexcept -> void
{

}

auto Emulator::reset() noexcept -> void
{ }

// Verifies that cartridge `data` is accurate and will operate on libgbemu, and
// sets the current cartridge to this data.
auto Emulator::cartridge(const std::vector<uint8_t>& data) -> void
{
    if (running)
    {
        pause();
    }

    try
    {
        m_cart = cart(data);
    }
    catch (std::runtime_error& err)
    {
        throw;
    }

    if (running)
    {
        reset();
    }
}