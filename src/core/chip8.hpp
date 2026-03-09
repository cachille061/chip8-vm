#pragma once

#include "types.hpp"

#include <array>
#include <cstdint>
#include <random>
#include <span>

namespace chip8 {

class Chip8 {
public:
    Chip8();

    /// Load a ROM into memory starting at ROM_START.
    /// Returns false if the ROM is too large.
    bool load_rom(std::span<const std::uint8_t> rom);

    /// Execute a single instruction cycle: fetch, decode, execute.
    void cycle();

    /// Decrement delay and sound timers. Call at 60 Hz.
    void tick_timers();

    // ---- State (public for testability and host access) ----

    std::array<std::uint8_t, MEMORY_SIZE> memory{};
    std::array<std::uint8_t, NUM_REGISTERS> V{};   // general-purpose registers V0–VF
    std::uint16_t I{};                               // index register
    std::uint16_t pc{ROM_START};                     // program counter
    std::array<std::uint16_t, STACK_SIZE> stack{};
    std::uint8_t sp{};                               // stack pointer

    std::uint8_t delay_timer{};
    std::uint8_t sound_timer{};

    std::array<std::uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT> display{};
    std::array<bool, NUM_KEYS> keypad{};

    bool draw_flag{};
    bool beep_flag{};
    bool waiting_for_key{};
    std::uint8_t key_register{};  // register to store key when waiting

private:
    std::mt19937 rng_{std::random_device{}()};
    // Built-in 4×5 font sprites (0–F), 80 bytes total.
    static constexpr std::array<std::uint8_t, 80> fontset{{
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    }};
};

} // namespace chip8
