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
// `#ifndef` to guard the contents of header files against multiple inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

#include <qmainwindow.h>

/// @brief Defines the main window.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Initializes the main window.
    MainWindow() noexcept;

protected:
    /// @brief Called when the user presses a key on the keyboard.
    /// @param event The key event data.
    void keyPressEvent(QKeyEvent* event);

    /// @brief Called when the user releases a key on the keyboard.
    /// @param event The key event data.
    void keyReleaseEvent(QKeyEvent* event);

private:
    // "File" menu
    struct
    {
        /// @brief The actual menu handle.
        QMenu* menu;

        /// @brief "Open ROM" menu item
        QAction* open_rom;
    } file;

signals:
    /// @brief Emitted when the user selects a ROM file to run.
    void rom_opened(const QString& file_name);

    /// @brief Emitted when a key has been pressed.
    void key_pressed(const int key);

    /// @brief Emitted when a key has been released.
    void key_released(const int key);
};