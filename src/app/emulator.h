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
// `#ifndef` to guard the contents of header files against multiple
// inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

#include <QThread>
#include "../libgbemu/include/gb.h"

/// @brief Defines the Game Boy system execution interface.
class Emulator : public QThread, public GameBoy::System
{
    Q_OBJECT

public:
    /// @brief Initializes the Game Boy system execution interface.
    Emulator() noexcept;

    /// @brief Starts the run loop. Does nothing if the loop is already
    /// running.
    auto start_run_loop() noexcept -> void;

    /// @brief Pauses the run loop, preserving the current state. Does nothing
    /// if the emulator is not running.
    auto pause_run_loop() noexcept -> void;

    /// @brief Generates a cartridge and sets it up to be executed.
    /// @param The data to use to generate the cartridge.
    auto cartridge(const std::vector<uint8_t>& data) -> void;

protected:
    /// @brief The starting point for the thread.
    void run() override;

private:
    /// @brief The maximum number of cycles per execution step.
    const unsigned int max_cycles{ 4194304 / 60 };

    /// @brief The number of cycles taken by the current execution step.
    unsigned int cycles{ 0 };

    /// @brief Is the emulator running?
    bool running{ false };

    /// @brief Current cartridge as generated by the `cartridge()` method.
    std::shared_ptr<GameBoy::Cartridge> m_cart;

signals:
    /// @brief Emitted when it is time to play audio samples.
    void play_audio(const std::vector<float>& samples);

    /// @brief Emitted when it is time to render a frame.
    void render_frame(const GameBoy::ScreenData& screen_data) noexcept;
};
