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

#include <qopenglwidget.h>
#include <QOpenGLFunctions_3_2_Core>
#include "../libgbemu/include/ppu.h"

/// @brief Defines an OpenGL 3.2 Core rendering context.
class OpenGL : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
{
    Q_OBJECT

public:
    /// @brief Renders screen data to the OpenGL context.
    /// @param screen_data The RGBA32 screen data to render.
    auto render_frame(const GameBoy::ScreenData& screen_data) noexcept -> void;

private:
    /// @brief Vertex buffer object
    GLuint VBO;

    /// @brief Vertex array object
    GLuint VAO;

    /// @brief Element buffer object
    GLuint EBO;

    /// @brief Texture ID
    GLuint texture;

    /// @brief Sets up the OpenGL resources and state. Gets called once before
    /// the first time `resizeGL()` or `paintGL()` is called.
    auto initializeGL() -> void;

    /// @brief Renders the OpenGL scene. Gets called whenever the widget needs
    /// to be updated.
    auto paintGL() -> void;

    /// @brief Sets up the OpenGL viewport, projection, etc. Gets called
    /// whenever the widget has been resized (and also when it is shown for the
    /// first time because all newly created widgets get a resize event
    /// automatically).
    /// @param w The width of the viewport.
    /// @param h The height of the viewport.
    auto resizeGL(int w, int h) -> void;
};