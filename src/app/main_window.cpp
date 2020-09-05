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

#include <qfiledialog.h>
#include <QKeyEvent>
#include <qmenubar.h>
#include "main_window.h"

/// @brief Initializes the main window.
MainWindow::MainWindow() noexcept
{
    setWindowTitle("gbemu");
    resize(640, 576);

    file.menu = menuBar()->addMenu(tr("&File"));
    file.open_rom = new QAction(tr("Open ROM..."), this);

    file.menu->addAction(file.open_rom);

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
}

/// @brief Called when the user presses a key on the keyboard.
/// @param event The key event data.
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    emit key_pressed(event->key());
}

/// @brief Called when the user releases key on the keyboard.
/// @param event The key event data.
void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    emit key_released(event->key());
}