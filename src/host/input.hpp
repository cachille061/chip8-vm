#pragma once

#include "core/types.hpp"

#include <SDL.h>
#include <array>

namespace chip8 {

/// Maps host keyboard events to the CHIP-8 hex keypad.
///
/// Layout mapping (QWERTY → CHIP-8):
///   1 2 3 4        1 2 3 C
///   Q W E R   →    4 5 6 D
///   A S D F        7 8 9 E
///   Z X C V        A 0 B F
class Input {
public:
    /// Process all pending SDL events.
    /// Updates `keypad` in-place and returns true if the user wants to quit.
    bool poll(std::array<bool, NUM_KEYS>& keypad);
};

} // namespace chip8
