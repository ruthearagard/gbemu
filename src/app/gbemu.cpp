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

#include <stdexcept>
#include <qfile.h>
#include <QAudioFormat>
#include <QAudioOutput>
#include <qmessagebox.h>
#include "gbemu.h"

/// @brief Initializes the main controller.
GBEmu::GBEmu() noexcept
{
    QAudioFormat fmt;

    fmt.setCodec("audio/pcm");
    fmt.setSampleSize(44100/60);
    fmt.setSampleRate(44100);
    fmt.setSampleType(QAudioFormat::Float);
    fmt.setByteOrder(QAudioFormat::LittleEndian);

    audio = new QAudioOutput(fmt);

    connect(&main_window, &MainWindow::rom_opened,
    [&](const QString& file_name)
    {
        QFile file(file_name);

        if (!file.open(QIODevice::ReadOnly))
        {
            const auto error_string
            {
                QString(tr("Unable to open %1: %2"))
                .arg(file_name).arg(file.errorString())
            };

            QMessageBox::critical(&main_window, tr("I/O error"), error_string);
            return;
        }

        const auto file_data{ file.readAll() };

        std::vector<uint8_t> rom_data(file_data.begin(), file_data.end());
        file.close();

        try
        {
            emulator.cartridge(rom_data);
        }
        catch (std::runtime_error& err)
        {
            const auto str
            {
                QString("gbemu cannot use this ROM: %1").arg(err.what())
            };
            QMessageBox::critical(&main_window, tr("Error"), str);
        }
    });

    connect(&main_window, &MainWindow::key_pressed, [&](const int key)
    {
        switch (key)
        {
            case Qt::Key::Key_Down:
                emulator.press_button(GameBoy::JoypadButton::Down);
                return;

            case Qt::Key::Key_Up:
                emulator.press_button(GameBoy::JoypadButton::Up);
                return;

            case Qt::Key::Key_Left:
                emulator.press_button(GameBoy::JoypadButton::Left);
                return;

            case Qt::Key::Key_Right:
                emulator.press_button(GameBoy::JoypadButton::Right);
                return;

            case Qt::Key::Key_Return:
                emulator.press_button(GameBoy::JoypadButton::Start);
                return;

            case Qt::Key::Key_Space:
                emulator.press_button(GameBoy::JoypadButton::Select);
                return;

            case Qt::Key::Key_A:
                emulator.press_button(GameBoy::JoypadButton::A);
                return;

            case Qt::Key::Key_B:
                emulator.press_button(GameBoy::JoypadButton::B);
                return;
        }
    });

    connect(&main_window, &MainWindow::key_released, [&](const int key)
    {
        switch (key)
        {
            case Qt::Key::Key_Down:
                emulator.release_button(GameBoy::JoypadButton::Down);
                return;

            case Qt::Key::Key_Up:
                emulator.release_button(GameBoy::JoypadButton::Up);
                return;

            case Qt::Key::Key_Left:
                emulator.release_button(GameBoy::JoypadButton::Left);
                return;

            case Qt::Key::Key_Right:
                emulator.release_button(GameBoy::JoypadButton::Right);
                return;

            case Qt::Key::Key_Return:
                emulator.release_button(GameBoy::JoypadButton::Start);
                return;

            case Qt::Key::Key_Space:
                emulator.release_button(GameBoy::JoypadButton::Select);
                return;

            case Qt::Key::Key_A:
                emulator.release_button(GameBoy::JoypadButton::A);
                return;

            case Qt::Key::Key_B:
                emulator.release_button(GameBoy::JoypadButton::B);
                return;
        }
    });

    connect(&emulator, &Emulator::play_audio, this,
    [&](const std::vector<float>& samples)
    {
        auto device{ audio->start() };
        device->write(reinterpret_cast<const char*>(samples.data()));
    });

    connect(&emulator, &Emulator::render_frame, this,
    [&](const GameBoy::ScreenData& screen_data)
    {
        opengl.render_frame(screen_data);
    });

    main_window.setCentralWidget(&opengl);
    main_window.show();
}
