#pragma once

#include <cstdint>
#include <cstddef>

namespace chip8 {

inline constexpr std::size_t MEMORY_SIZE    = 4096;
inline constexpr std::size_t NUM_REGISTERS  = 16;
inline constexpr std::size_t STACK_SIZE     = 16;
inline constexpr std::size_t DISPLAY_WIDTH  = 64;
inline constexpr std::size_t DISPLAY_HEIGHT = 32;
inline constexpr std::size_t NUM_KEYS       = 16;
inline constexpr std::uint16_t FONTSET_START = 0x050;
inline constexpr std::uint16_t ROM_START     = 0x200;
inline constexpr std::size_t MAX_ROM_SIZE   = MEMORY_SIZE - ROM_START;

} // namespace chip8
