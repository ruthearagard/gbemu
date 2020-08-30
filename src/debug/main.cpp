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

// Required for time functions.
#include <chrono>

// Required for file I/O streams.
#include <fstream>

// Required for I/O streams.
#include <iostream>

// Required for SDL2.
#include <SDL2/SDL.h>

// Required for the `GameBoy::System` class.
#include "../libgbemu/include/gb.h"

// Required for the `Disassembler` class.
#include "disasm.h"

static auto open_file_and_read
(const std::string& file) noexcept -> std::vector<uint8_t>
{
    std::basic_ifstream<unsigned char> handle
    {
        file,            // File path
        std::ios::binary // Open mode
    };

    if (!handle)
    {
        return { };
    }

    std::vector<uint8_t> data
    {
        std::istreambuf_iterator<unsigned char>{handle}, // File contents
        {}
    };

    handle.close();
    return data;
}

// Program entry point.
auto main(int argc, char* argv[]) -> int
{
    if (argc < 2)
    {
        std::cerr << "Missing required argument(s)." << std::endl;

        std::cerr << "Syntax: " << argv[0] << " cart_file [boot_rom]"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> boot_rom;

    if (argc > 2)
    {
        boot_rom = open_file_and_read(argv[2]);

        if (boot_rom.empty())
        {
            std::cerr << "Unable to open " << argv[2] << ": "
                      << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Using boot ROM: " << argv[2] << std::endl;
    }

    const auto cart_data{ open_file_and_read(argv[1]) };

    if (cart_data.empty())
    {
        std::cerr << argv[0] << ": Unable to open " << argv[2] << ": "
                  << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Using cartridge file: " << argv[1] << std::endl;

    GameBoy::System gb;
    std::shared_ptr<GameBoy::Cartridge> cart;

    try
    {
        cart = gb.cart(cart_data);
    }
    catch (std::runtime_error& err)
    {
        std::cerr << "Unable to use cartridge file: " << err.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Cartridge title: "    << cart->title()    << std::endl;
    std::cout << "Cartridge type: "     << cart->type()     << std::endl;
    std::cout << "Cartridge ROM size: " << cart->rom_size() << std::endl;
    std::cout << "Cartridge RAM size: " << cart->ram_size() << std::endl;

    if (!boot_rom.empty())
    {
        gb.boot_rom(boot_rom);
    }

    Disassembler disasm
    {
        gb.bus, // System bus
        gb.cpu  // Sharp SM83 CPU interpreter
    };

    std::ofstream trace_file{ "trace.txt" };

    if (!trace_file)
    {
        std::cerr << "Unable to open trace file for writing: "
                  << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_Window* window = SDL_CreateWindow("gbemu debugging station",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          640,
                                          576,
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Renderer* renderer = SDL_CreateRenderer(window,
                                                -1,
                                                SDL_RENDERER_ACCELERATED);

    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_BGRA32,
                                             SDL_TEXTUREACCESS_TARGET,
                                             GameBoy::PPU::ScreenX,
                                             GameBoy::PPU::ScreenY);

    bool done{ false };
    SDL_Event event;

    const unsigned int max_cycles{ 4194304 / 60 };
    unsigned int cycles{ 0 };

    while (!done)
    {
        SDL_PollEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                done = true;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(window))
                {
                    done = true;
                    break;
                }
                break;
        }

        const auto start{ std::chrono::steady_clock::now() };
            while (cycles < max_cycles)
            {
                //disasm.before();
                cycles += gb.step();
                //trace_file << disasm.after() << std::endl;
            }

            cycles -= max_cycles;

            SDL_UpdateTexture(texture,
                              nullptr,
                              gb.bus.ppu.screen_data.data(),
                              sizeof(uint32_t) * GameBoy::PPU::ScreenX);

            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        const auto end{ std::chrono::steady_clock::now() };

        const auto diff
        {
            std::chrono::duration_cast<std::chrono::milliseconds>
            (end - start).count()
        };

        if (diff < (1000 / 60))
        {
            SDL_Delay((1000 / 60) - diff);
        }
    }
    return EXIT_SUCCESS;
}