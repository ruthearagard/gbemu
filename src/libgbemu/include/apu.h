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

#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace GameBoy
{
    /// @brief Defines the audio processing unit (APU).
    class APU
    {
    public:
        union length_duty_data
        {
            struct
            {
                /// @brief Bit 5-0: Sound length data (Write Only) (t1: 0-63)
                unsigned int length : 6;

                /// @brief Bit 7-6 - Wave Pattern Duty (Read/Write)
                unsigned int duty : 2;
            };
            uint8_t byte;
        };

        union volume_envelope_data
        {
            struct
            {
                /// @brief Bit 2-0: Number of envelope sweep (n: 0-7)
                /// (If zero, stop envelope operation.)
                unsigned int shift_amount : 3;

                /// @brief Bit 3: Envelope Direction (0=Decrease, 1=Increase)
                unsigned int negate : 1;

                /// @brief Bit 7-4: Initial Volume of envelope ($0-$0F)
                /// (0=No Sound)
                unsigned int starting_volume : 4;
            };
            uint8_t byte;
        };

        union freq_hi_data
        {
            struct
            {
                /// @brief Bit 2-0: Frequency's higher 3 bits
                unsigned int hi : 3;

                unsigned int : 3;

                /// @brief Bit 6: Counter/consecutive selection
                /// (1 = Stop output when length in NR11 expires)
                unsigned int counter : 1;

                /// @brief Bit 7: Initial (1=Restart Sound)
                unsigned int trigger : 1;
            };
            uint8_t byte;
        };

        /// @brief Sound Channel 1 - Tone & Sweep
        struct
        {
            /// @brief $FF10 - NR10 - Channel 1 Sweep register (R/W)
            union
            {
                struct
                {
                    /// @brief Bit 2-0: Number of sweep shift (n: 0-7)
                    unsigned int shift : 1;

                    /// @brief Bit 6-4: Sweep Time
                    unsigned int duration : 3;

                    /// @brief Bit 3: Sweep Increase/Decrease
                    /// 0: Addition(frequency increases)
                    /// 1: Subtraction(frequency decreases)
                    unsigned int negate : 1;

                    /// @brief Bit 4-6: Sweep time
                    unsigned int time : 3;
                };
                uint8_t byte;
            } NR10;

            /// @brief $FF11 - NR11 - Channel 1 Sound length/Wave pattern duty (R/W)
            union length_duty_data NR11;

            /// @brief $FF12 - NR12 - Channel 1 Volume Envelope (R/W)
            union volume_envelope_data NR12;

            /// @brief $FF13 - NR13 - Channel 1 Frequency lo (W)
            uint8_t NR13;

            /// @brief $FF14 - NR14 - Channel 1 Frequency hi (R/W)
            union freq_hi_data NR14;
        } CH1;

        /// @brief Sound Channel 2 - Tone
        ///
        /// This sound channel works exactly as channel 1, except that it
        /// doesn't have a Tone Envelope/Sweep Register.
        struct
        {
            /// @brief $FF16 - NR21 - Channel 2 Sound Length/
            /// Wave Pattern Duty (R/W)
            union length_duty_data NR21;

            /// @brief $FF17 - NR22 - Channel 2 Volume Envelope (R/W)
            union volume_envelope_data NR22;

            /// @brief $FF18 - NR23 - Channel 2 Frequency lo data (W)
            uint8_t NR23;

            /// @brief $FF19 - NR24 - Channel 2 Frequency hi data (R/W)
            union freq_hi_data NR24;
        } CH2;

        /// @brief Sound Channel 3 - Wave Output
        ///
        /// This channel can be used to output digital sound, the length of the
        /// sample buffer (Wave RAM) is limited to 32 digits. This sound
        /// channel can be also used to output normal tones when initializing
        /// the Wave RAM by a square wave. This channel doesn't have a volume
        /// envelope register.
        struct
        {
            /// @brief $FF1A - NR30 - Channel 3 Sound on/off (R/W)
            union
            {
                struct
                {
                    unsigned int : 6;

                    /// @brief Bit 7: Sound Channel 3 Off (0=Stop, 1=Playback)
                    unsigned int enabled : 1;
                };
                uint8_t byte;
            } NR30;

            /// @brief $FF1B - NR31 - Channel 3 Sound Length
            uint8_t NR31;

            // @brief $FF1C - NR32 - Channel 3 Select output level (R/W)
            union
            {
                struct
                {
                    unsigned int : 5;

                    /// @brief Bit 6-5: Select output level (R/W)
                    ///
                    /// Possible Output levels are:
                    /// 
                    /// 0: Mute (No sound)
                    /// 1: 100% Volume (Wave Pattern RAM Data >> 0)
                    /// 2: 50% Volume (Wave Pattern RAM data >> 1)
                    /// 3: 25% Volume (Wave Pattern RAM data >> 2)
                    unsigned int output_level : 2;

                    unsigned int : 1;
                };
                uint8_t byte;
            } NR32;

            /// @brief $FF1D - NR33 - Channel 3 Frequency's lower data (W)
            uint8_t NR33;

            /// @brief $FF1E - NR34 - Channel 3 Frequency's higher data (R/W)
            union freq_hi_data NR34;

            /// @brief [$FF30 - $FF3F]: Wave Pattern RAM
            ///
            /// This storage area holds 32 4-bit samples that are played back
            /// upper 4 bits first.
            std::array<uint8_t, 16> ram;
        } CH3;

        /// @brief Sound Channel 4 - Noise
        ///
        /// This channel is used to output white noise. This is done by
        /// randomly switching the amplitude between high and low at a given
        /// frequency. Depending on the frequency the noise will appear
        /// 'harder' or 'softer'.
        ///
        /// It is also possible to influence the function of the random
        /// generator, so the that the output becomes more regular, resulting
        /// in a limited ability to output Tone instead of Noise.
        struct
        {
            /// @brief $FF20 - NR41 - Channel 4 Sound Length (R/W)
            union
            {
                struct
                {
                    /// @brief Sound length data (t1: 0-63)
                    unsigned int length : 6;

                    unsigned int : 2;
                };
                uint8_t byte;
            } NR41;

            /// @brief $FF21 - NR42 - Channel 4 Volume Envelope (R/W)
            union volume_envelope_data NR42;

            /// @brief $FF22 - NR43 - Channel 4 Polynomial Counter (R/W)
            /// 
            /// The amplitude is randomly switched between high and low at the
            /// given frequency. A higher frequency will make the noise to
            /// appear 'softer'.
            /// 
            /// When Bit 3 is set, the output will become more regular, and
            /// some frequencies will sound more like Tone than Noise.
            union
            {
                struct
                {
                    /// @brief Bit 2-0: Dividing Ratio of Frequencies (r)
                    unsigned int div_ratio : 3;

                    /// @brief Bit 3: Counter Step/Width (0=15 bits, 1=7 bits)
                    unsigned int width : 1;

                    /// @brief Bit 7-4: Shift Clock Frequency (s)
                    unsigned int clock_freq : 4;
                };
                uint8_t byte;
            } NR43;

            /// @brief $FF23 - NR44 - Channel 4 Counter/consecutive; Inital (R/W)
            union
            {
                struct
                {
                    unsigned int : 6;

                    /// @brief Bit 6 - Counter/consecutive selection (R/W)
                    /// (1 = Stop output when length in NR41 expires)
                    unsigned int counter : 1;

                    /// @brief Bit 7: Initial (1=Restart Sound)
                    unsigned int trigger : 1;
                };
                uint8_t byte;
            } NR44;
        } CH4;

        /// @brief $FF24 - NR50 - Channel control/ON-OFF/Volume (R/W)
        union
        {
            struct
            {
                /// @brief Bit 2-0: SO1 output level (volume) (0-7)
                unsigned int so1_output_level : 3;

                /// @brief Bit 3: Output Vin to SO1 terminal (1=Enable)
                unsigned int output_vin_to_so1 : 1;

                /// @brief Bit 6-4: SO2 output level (volume) (0-7)
                unsigned int so2_output_level : 3;

                /// @brief Bit 7: Output Vin to SO2 terminal (1=Enable)
                unsigned int output_vin_to_so2 : 1;
            };
            uint8_t byte;
        } NR50;

        /// @brief $FF25 - NR51 - Selection of Sound output terminal (R/W)
        union
        {
            struct
            {
                /// @brief Bit 0 - Output sound 1 to SO1 terminal
                unsigned int output_ch1_to_so1 : 1;

                /// @brief Bit 1 - Output sound 2 to SO1 terminal
                unsigned int output_ch2_to_so1 : 1;

                /// @brief Bit 2 - Output sound 3 to SO1 terminal
                unsigned int output_ch3_to_so1 : 1;

                /// @brief Bit 3 - Output sound 4 to SO1 terminal
                unsigned int output_ch4_to_so1 : 1;

                /// @brief Bit 4 - Output sound 1 to SO2 terminal
                unsigned int output_ch1_to_so2 : 1;

                /// @brief Bit 5 - Output sound 2 to SO2 terminal
                unsigned int output_ch2_to_so2 : 1;

                /// @brief Bit 6 - Output sound 3 to SO2 terminal
                unsigned int output_ch3_to_so2 : 1;

                /// @brief Bit 7 - Output sound 4 to SO2 terminal
                unsigned int output_ch4_to_so2 : 1;
            };
            uint8_t byte;
        } NR51;

        /// @brief $FF26 - NR52 - Sound on/off
        union
        {
            struct
            {
                /// @brief Bit 0 - Sound 1 ON flag (R)
                unsigned int ch1_on : 1;

                /// @brief Bit 1 - Sound 2 ON flag (R)
                unsigned int ch2_on : 1;

                /// @brief Bit 2 - Sound 3 ON flag (R)
                unsigned int ch3_on : 1;

                /// @brief Bit 3 - Sound 4 ON flag (R)
                unsigned int ch4_on : 1;

                unsigned int : 3;

                /// @brief Bit 7 - All sound on/off (R/W)
                unsigned int enabled : 1;
            };
            uint8_t byte;
        } NR52;

        unsigned int frame_sequencer;
        unsigned int frame_sequencer_step;

        /// @brief Sets a register.
        /// @param data
        /// @return the data
        auto set_register_check(const uint8_t data) noexcept -> uint8_t;

        /// @brief Sets wave RAM data.
        /// @param address The index of the wave RAM data.
        /// @param data The data to place in the wave RAM index.
        auto set_wave_ram(const uint16_t address,
                          const uint8_t data) noexcept -> void;

        /// @brief Sets the $FF14 - NR14 - Channel 1 Frequency hi (R/W)
        /// register.
        /// @param data The data to set the register to.
        auto set_NR14(const uint8_t data) noexcept -> void;

        /// @brief Sets the $FF19 - NR24 - Channel 2 Frequency hi data (R/W)
        /// register.
        /// @param data The data to set the register to.
        auto set_NR24(const uint8_t data) noexcept -> void;

        /// @brief Sets the $FF1E - NR34 - Channel 3 Frequency's higher data
        /// (R/W) register.
        /// @param data The data to set the register to.
        auto set_NR34(const uint8_t data) noexcept -> void;

        /// @brief Sets the $FF23 - NR44 - Channel 4 Counter/consecutive;
        /// Inital (R/W).
        /// @param data The data to set the register to.
        auto set_NR44(const uint8_t data) noexcept -> void;

        /// @brief Sets the $FF26 - NR52 - Sound on/off register.
        /// @param data The data to set the register to.
        auto set_NR52(const uint8_t data) noexcept -> void;

        /// @brief Resets the APU to the startup state.
        auto reset() noexcept -> void;

        /// @brief Steps the APU by 1 m-cycle.
        auto step() noexcept -> void;

        std::vector<float> samples;
    };
}
