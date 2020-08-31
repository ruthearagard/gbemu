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

// Required for the `OpenGL` class.
#include "opengl.h"

auto OpenGL::initializeGL() -> void
{
    QSurfaceFormat fmt;
    fmt.setVersion(3, 2);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);
    QSurfaceFormat::setDefaultFormat(fmt);

    initializeOpenGLFunctions();

    const auto vertex_shader_src
    {
        R"(
        #version 150 core
        in vec2 position;
        in vec2 texcoord;

        out vec2 Texcoord;

        void main()
        {
            Texcoord = texcoord;
            gl_Position = vec4(position, 0.0, 1.0);
        })"
    };

    const auto fragment_shader_src
    {
        R"(
        #version 150 core

        in vec2 Texcoord;
        uniform sampler2D tex;
        out vec4 outColor;
    
        void main()
        {
            outColor = texture(tex, Texcoord);
        })"
    };

    const auto vertex_shader  { glCreateShader(GL_VERTEX_SHADER)   };
    const auto fragment_shader{ glCreateShader(GL_FRAGMENT_SHADER) };

    glShaderSource(vertex_shader, 1, &vertex_shader_src, nullptr);
    glCompileShader(vertex_shader);

    glShaderSource(fragment_shader, 1, &fragment_shader_src, nullptr);
    glCompileShader(fragment_shader);

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    constexpr float vertices[] =
    {
    // Position      Texture Coordinates
       1.0F,  1.0F, 1.0F, 0.0F,
      -1.0F,  1.0F, 0.0F, 0.0F,
       1.0F, -1.0F, 1.0F, 1.0F,
      -1.0F, -1.0F, 0.0F, 1.0F,
    };

    constexpr unsigned int indices[] =
    {
        0, 1, 3,
        2, 3, 0
    };

    const auto program{ glCreateProgram() };
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices), indices, GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          4 * sizeof(float),
                          reinterpret_cast<void*>(2 * sizeof(float)));

    glEnableVertexAttribArray(1);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 GameBoy::ScreenX,
                 GameBoy::ScreenY,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_BYTE,
                 nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

auto OpenGL::paintGL() -> void
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto OpenGL::resizeGL(int w, int h) -> void
{
    glViewport(0, 0, w, h);
}

auto OpenGL::render_frame(const GameBoy::ScreenData& screen_data) noexcept -> void
{
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    GameBoy::ScreenX,
                    GameBoy::ScreenY,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    screen_data.data());
}