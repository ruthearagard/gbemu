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

// Required for `std::map<>`.
#include <map>

// Required for `std::string`.
#include <string>

// Required for `std::vector<>`.
#include <vector>

namespace GameBoy
{
    class Cartridge
    {
    public:
        // Returns data from the cartridge referenced by memory address
        // `address`.
        virtual auto read(const uint16_t address) -> uint8_t = 0;

        // Updates the memory bank controller configuration `address` to
        // `value`.
        virtual auto write(const uint16_t address,
                           const uint8_t data) -> void = 0;

        // Returns the title of the cartridge.
        auto title() noexcept -> std::string
        {
            std::string result;

            for (auto index{ 0x0134 }; index < 0x0143; ++index)
            {
                result += m_data[index];
            }
            return result;
        }

        // Returns the type of cartridge.
        auto type() noexcept -> std::string
        {
            return types.at(m_data[0x0147]);
        }

        // Returns the ROM size of the cartridge.
        auto rom_size() noexcept -> std::string
        {
            return rom_sizes.at(m_data[0x0148]);
        }

        // Returns the RAM size of the cartridge.
        auto ram_size() noexcept -> std::string
        {
            return ram_sizes.at(m_data[0x0149]);
        }

        static const inline std::map<const uint8_t, const std::string> types =
        {
            { 0x00, "ROM ONLY"                },
            { 0x01, "MBC1"                    },
            { 0x02, "MBC1+RAM"                },
            { 0x03, "MBC1+RAM+BATTERY"        },
            { 0x05, "MBC2"                    },
            { 0x06, "MBC2+BATTERY"            },
            { 0x08, "ROM+RAM"                 },
            { 0x09, "ROM+RAM+BATTERY"         },
            { 0x0B, "MMM01"                   },
            { 0x0C, "MMM01+RAM"               },
            { 0x0D, "MMM01+RAM+BATTERY"       },
            { 0x0F, "MBC3+TIMER+BATTERY"      },
            { 0x10, "MBC3+TIMER+RAM+BATTERY"  },
            { 0x11, "MBC3"                    },
            { 0x12, "MBC3+RAM"                },
            { 0x13, "MBC3+RAM+BATTERY"        },
            { 0x15, "MBC4"                    },
            { 0x16, "MBC4+RAM"                },
            { 0x17, "MBC4+RAM+BATTERY"        },
            { 0x19, "MBC5"                    },
            { 0x1A, "MBC5+RAM"                },
            { 0x1B, "MBC5+RAM+BATTERY"        },
            { 0x1C, "MBC5+RUMBLE"             },
            { 0x1D, "MBC5+RUMBLE+RAM"         },
            { 0x1E, "MBC5+RUMBLE+RAM+BATTERY" },
            { 0xFC, "POCKET CAMERA"           },
            { 0xFD, "BANDAI TAMA5"            },
            { 0xFE, "HuC3"                    },
            { 0xFF, "HuC1+RAM+BATTERY"        }
        };

        static const inline std::map<const uint8_t,
                                     const std::string> rom_sizes =
        {
            { 0x00, "32KB (no ROM banking)"                         },
            { 0x01, "64KB (4 banks)"                                },
            { 0x02, "128KB (8 banks)"                               },
            { 0x03, "256KB (16 banks)"                              },
            { 0x04, "512KB (32 banks)"                              },
            { 0x05, "1MB (64 banks) - only 63 banks used by MBC1"   },
            { 0x06, "2MB (128 banks) - only 125 banks used by MBC1" },
            { 0x07, "4MB (256 banks)"                               },
            { 0x52, "1.1MB (72 banks)"                              },
            { 0x53, "1.2MB (80 banks)"                              },
            { 0x54, "1.5MB (96 banks)"                              }
        };

        static const inline std::map<const uint8_t,
                                     const std::string> ram_sizes =
        {
            { 0x00, "None"                        },
            { 0x01, "2 KB"                        },
            { 0x02, "8 KB"                        },
            { 0x03, "32 KB (4 banks of 8KB each)" }
        };

    protected:
        explicit Cartridge
        (const std::vector<uint8_t>& data) noexcept
        {
            m_data = data;
        }

        std::vector<uint8_t> m_data;
    };
}