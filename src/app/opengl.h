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
// ‘#ifndef’ to guard the contents of header files against multiple inclusions."
//
// Source: https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html
#pragma once

// Required for the `QOpenGLWidget` class.
#include <qopenglwidget.h>

// Required for the `QOpenGLFunctions_3_2_Core` class.
#include <QOpenGLFunctions_3_2_Core>

#include "../libgbemu/include/ppu.h"

class OpenGL : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
{
    Q_OBJECT

public:
    auto render_frame(const GameBoy::ScreenData& screen_data) noexcept -> void;

private:
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;

    GLuint texture;

    auto initializeGL() -> void;
    auto paintGL() -> void;
    auto resizeGL(int w, int h) -> void;
};