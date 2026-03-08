#include "chip8.hpp"

#include <algorithm>

namespace chip8 {

Chip8::Chip8()
{
    // Load fontset into memory
    std::copy(fontset.begin(), fontset.end(), memory.begin() + FONTSET_START);
}

bool Chip8::load_rom(std::span<const std::uint8_t> rom)
{
    if (rom.size() > MAX_ROM_SIZE) {
        return false;
    }
    std::copy(rom.begin(), rom.end(), memory.begin() + ROM_START);
    return true;
}

void Chip8::cycle()
{
    // TODO: Phase 2 — fetch, decode, execute
}

void Chip8::tick_timers()
{
    if (delay_timer > 0) {
        --delay_timer;
    }
    if (sound_timer > 0) {
        --sound_timer;
        beep_flag = true;
    } else {
        beep_flag = false;
    }
}

} // namespace chip8
