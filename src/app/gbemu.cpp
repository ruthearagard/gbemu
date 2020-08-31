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

// Required for the `QFile` class.
#include <qfile.h>

// Required for the `QMessageBox` class.
#include <qmessagebox.h>

// Required for the `GBEmu` class.
#include "gbemu.h"

GBEmu::GBEmu() noexcept
{
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
        emulator.start();
    });

    connect(&main_window, &MainWindow::preferences, [&]()
    {
        preferences = new Preferences();
        preferences->setAttribute(Qt::WA_DeleteOnClose);
        preferences->setWindowModality(Qt::ApplicationModal);

        preferences->show();
    });

    connect(&main_window, &MainWindow::on_display_log, [&]()
    {
        if (!message_logger)
        {
            message_logger = new MessageLogger(&main_window);
            message_logger->setAttribute(Qt::WA_DeleteOnClose);
        }
        message_logger->show();
    });

    connect(&emulator, &Emulator::render_frame,
    [&](const GameBoy::ScreenData& screen_data)
    {
        opengl.render_frame(screen_data);
    });

    main_window.setCentralWidget(&opengl);
    main_window.show();
}