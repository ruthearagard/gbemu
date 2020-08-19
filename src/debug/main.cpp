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

// Required for file I/O streams.
#include <fstream>

// Required for I/O streams.
#include <iostream>

// Required for the `GameBoy::System` class.
#include "../libgbemu/include/gb.h"

// Required for the `Disassembler` class.
#include "disasm.h"

// Program entry point.
auto main(int argc, char* argv[]) -> int
{
    if (argc < 2)
    {
        std::cerr << argv[0] << ": Missing required argument. " << std::endl;
        std::cerr << argv[0] << ": Syntax: " << argv[0] << " cart_file"
                  << std::endl;

        return EXIT_FAILURE;
    }

    std::ifstream cart_file
    {
        argv[1],         // File path
        std::ios::binary // Open mode
    };

    if (!cart_file)
    {
        std::cerr << argv[0] << ": Unable to open " << argv[1] << ": "
                  << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> cart_data;

    std::copy(std::istreambuf_iterator<char>{cart_file},
              std::istreambuf_iterator<char>{},
              std::back_inserter(cart_data));

    cart_file.close();

    std::shared_ptr<GameBoy::Cartridge> cart;

    try
    {
        cart = std::make_shared<GameBoy::Cartridge>(cart_data);
    }
    catch (std::runtime_error& err)
    {
        std::cerr << argv[0] << ": Unable to use cartridge file " << argv[1]
                  << ": " << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    GameBoy::System gb;
    gb.cart(cart);

    Disassembler disasm
    {
        gb.bus, // System bus
        gb.cpu  // Sharp SM83 CPU interpreter
    };

    std::ofstream trace_file{ "trace.txt" };

    if (!trace_file)
    {
        std::cerr << argv[0] << ": Unable to open trace file for writing: "
                  << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    for (;;)
    {
        disasm.before();
        gb.step();
        trace_file << disasm.after() << std::endl;
    }
    return EXIT_SUCCESS;
}