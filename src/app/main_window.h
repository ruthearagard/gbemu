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
// �#ifndef� to guard the contents of header files against multiple inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

// Required for the `QMainWindow` class.
#include <qmainwindow.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow() noexcept;

private:
    // "File" menu
    struct
    {
        QMenu* menu;
        QAction* open_rom;
    } file;

    // "Edit" menu
    struct
    {
        QMenu* menu;
        QAction* preferences;
    } edit;

    // "Debug" menu
    struct
    {
        QMenu* menu;
        QAction* display_log;
        QAction* display_serial_output;
        QAction* cpu;
        QAction* memory_viewer;
        QAction* ppu;
        QAction* timer;
    } debug;

signals:
    // Emitted when the user selects a ROM file to run.
    void rom_opened(const QString& file_name) noexcept;

    // Emitted when the user requests to change application preferences.
    void preferences() noexcept;

    // Emitted when the user requests to display the log.
    void on_display_log() noexcept;

    // Emitted when the user requests to display serial output.
    void on_display_serial_output() noexcept;

    // Emitted when the user requests to display the CPU debugger.
    void on_cpu_debugger() noexcept;
};