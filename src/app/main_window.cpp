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

// Required for the `QFileDialog` class.
#include <qfiledialog.h>

// Required for the `QMenuBar` class.
#include <qmenubar.h>

// Required for the `MainWindow` class.
#include "main_window.h"

MainWindow::MainWindow() noexcept
{
    setWindowTitle("gbemu");
    resize(640, 576);

    file.menu = menuBar()->addMenu(tr("&File"));
    file.open_rom = new QAction(tr("Open ROM..."), this);

    file.menu->addAction(file.open_rom);

    edit.menu = menuBar()->addMenu(tr("&Edit"));
    edit.preferences = new QAction(tr("Preferences..."), this);

    edit.menu->addAction(edit.preferences);

    debug.menu = menuBar()->addMenu(tr("&Debug"));
    debug.display_log = new QAction(tr("Display log..."), this);

    debug.display_serial_output =
    new QAction(tr("Display serial output..."), this);

    debug.memory_viewer = new QAction("Memory Viewer", this);
    debug.ppu           = new QAction("PPU", this);
    debug.timer         = new QAction("Timer", this);

    debug.menu->addAction(debug.display_log);
    debug.menu->addAction(debug.display_serial_output);
    debug.menu->addAction(debug.memory_viewer);
    debug.menu->addAction(debug.ppu);
    debug.menu->addAction(debug.timer);

    connect(file.open_rom, &QAction::triggered, [&]()
    {
        const auto file_name
        {
            QFileDialog::getOpenFileName(this,
                                         tr("Open Game Boy ROM"),
                                         "",
                                         tr("Game Boy ROMs (*.gb)"))
        };

        if (!file_name.isEmpty())
        {
            emit rom_opened(file_name);
        }
    });

    connect(edit.preferences, &QAction::triggered, [&]()
    {
        emit preferences();
    });

    connect(debug.display_log, &QAction::triggered, [&]()
    {
        emit on_display_log();
    });

    connect(debug.display_serial_output, &QAction::triggered, [&]()
    {
        emit on_display_serial_output();
    });
}