// Copyright 2020 Michael Rodriguez
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

// XXX: NONSTANDARD
//
// "If #pragma once is seen when scanning a header file, that file will never
// be read again, no matter what. It is a less-portable alternative to using
// ‘#ifndef’ to guard the contents of header files against multiple inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

// Required for the `QMainWindow` class.
#include <qmainwindow.h>

// Forward declaration
class QPlainTextEdit;

class MessageLogger : public QMainWindow
{
    Q_OBJECT

public:
    explicit MessageLogger(QWidget* parent) noexcept;

    auto info(const QString& msg) noexcept -> void;
    auto warning(const QString& msg) noexcept -> void;

    auto append(const QString& data) -> void;
    auto reset() -> void;

private:
    auto on_select_font() -> void;
    auto on_save_log() -> void;

    struct
    {
        QMenu* menu;
        QAction* save;
    } file;

    struct
    {
        QMenu* menu;
        QAction* select_font;
        QAction* clear_log;
    } view;

    QPlainTextEdit* text_edit;
};