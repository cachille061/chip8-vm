#include "chip8.hpp"

#include <algorithm>
#include <random>

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
    // If waiting for a key press (Fx0A), check keypad each cycle
    if (waiting_for_key) {
        for (std::uint8_t i = 0; i < NUM_KEYS; ++i) {
            if (keypad[i]) {
                V[key_register] = i;
                waiting_for_key = false;
                break;
            }
        }
        return;
    }

    // Fetch: read 2-byte big-endian opcode
    std::uint16_t opcode = (static_cast<std::uint16_t>(memory[pc]) << 8) | memory[pc + 1];
    pc += 2;

    // Decode helpers
    std::uint8_t  x   = (opcode >> 8) & 0x0F;
    std::uint8_t  y   = (opcode >> 4) & 0x0F;
    std::uint8_t  kk  = opcode & 0x00FF;
    std::uint16_t nnn = opcode & 0x0FFF;
    std::uint8_t  n   = opcode & 0x000F;

    switch (opcode & 0xF000) {

    case 0x0000:
        switch (opcode) {
        case 0x00E0: // CLS — clear display
            display.fill(0);
            draw_flag = true;
            break;
        case 0x00EE: // RET — return from subroutine
            --sp;
            pc = stack[sp];
            break;
        default:
            break; // 0nnn (SYS addr) ignored on modern interpreters
        }
        break;

    case 0x1000: // 1nnn — JP addr
        pc = nnn;
        break;

    case 0x2000: // 2nnn — CALL addr
        stack[sp] = pc;
        ++sp;
        pc = nnn;
        break;

    case 0x3000: // 3xkk — SE Vx, byte
        if (V[x] == kk) pc += 2;
        break;

    case 0x4000: // 4xkk — SNE Vx, byte
        if (V[x] != kk) pc += 2;
        break;

    case 0x5000: // 5xy0 — SE Vx, Vy
        if (V[x] == V[y]) pc += 2;
        break;

    case 0x6000: // 6xkk — LD Vx, byte
        V[x] = kk;
        break;

    case 0x7000: // 7xkk — ADD Vx, byte (no carry flag)
        V[x] += kk;
        break;

    case 0x8000: // 8xy_ — Arithmetic/logic
        switch (n) {
        case 0x0: // LD Vx, Vy
            V[x] = V[y];
            break;
        case 0x1: // OR Vx, Vy
            V[x] |= V[y];
            V[0xF] = 0;
            break;
        case 0x2: // AND Vx, Vy
            V[x] &= V[y];
            V[0xF] = 0;
            break;
        case 0x3: // XOR Vx, Vy
            V[x] ^= V[y];
            V[0xF] = 0;
            break;
        case 0x4: { // ADD Vx, Vy (with carry)
            std::uint16_t sum = V[x] + V[y];
            V[x] = static_cast<std::uint8_t>(sum);
            V[0xF] = (sum > 0xFF) ? 1 : 0;
            break;
        }
        case 0x5: { // SUB Vx, Vy (Vx = Vx - Vy)
            std::uint8_t flag = (V[x] >= V[y]) ? 1 : 0;
            V[x] -= V[y];
            V[0xF] = flag;
            break;
        }
        case 0x6: { // SHR Vx {, Vy} — shift Vy right, store in Vx
            std::uint8_t lsb = V[y] & 0x1;
            V[x] = V[y] >> 1;
            V[0xF] = lsb;
            break;
        }
        case 0x7: { // SUBN Vx, Vy (Vx = Vy - Vx)
            std::uint8_t flag = (V[y] >= V[x]) ? 1 : 0;
            V[x] = V[y] - V[x];
            V[0xF] = flag;
            break;
        }
        case 0xE: { // SHL Vx {, Vy} — shift Vy left, store in Vx
            std::uint8_t msb = (V[y] >> 7) & 0x1;
            V[x] = V[y] << 1;
            V[0xF] = msb;
            break;
        }
        default:
            break;
        }
        break;

    case 0x9000: // 9xy0 — SNE Vx, Vy
        if (V[x] != V[y]) pc += 2;
        break;

    case 0xA000: // Annn — LD I, addr
        I = nnn;
        break;

    case 0xB000: // Bnnn — JP V0, addr
        pc = nnn + V[0];
        break;

    case 0xC000: { // Cxkk — RND Vx, byte
        std::uniform_int_distribution<unsigned> dist(0, 255);
        V[x] = static_cast<std::uint8_t>(dist(rng_)) & kk;
        break;
    }

    case 0xD000: { // Dxyn — DRW Vx, Vy, nibble
        std::uint8_t xpos = V[x] % DISPLAY_WIDTH;
        std::uint8_t ypos = V[y] % DISPLAY_HEIGHT;
        V[0xF] = 0;

        for (unsigned row = 0; row < n; ++row) {
            if (ypos + row >= DISPLAY_HEIGHT) break;
            std::uint8_t sprite_byte = memory[I + row];
            for (unsigned col = 0; col < 8; ++col) {
                if (xpos + col >= DISPLAY_WIDTH) break;
                if ((sprite_byte & (0x80 >> col)) != 0) {
                    std::size_t idx = (ypos + row) * DISPLAY_WIDTH + (xpos + col);
                    if (display[idx] == 1) {
                        V[0xF] = 1;
                    }
                    display[idx] ^= 1;
                }
            }
        }
        draw_flag = true;
        break;
    }

    case 0xE000:
        switch (kk) {
        case 0x9E: // Ex9E — SKP Vx
            if (keypad[V[x] & 0xF]) pc += 2;
            break;
        case 0xA1: // ExA1 — SKNP Vx
            if (!keypad[V[x] & 0xF]) pc += 2;
            break;
        default:
            break;
        }
        break;

    case 0xF000:
        switch (kk) {
        case 0x07: // Fx07 — LD Vx, DT
            V[x] = delay_timer;
            break;
        case 0x0A: // Fx0A — LD Vx, K (wait for key)
            waiting_for_key = true;
            key_register = x;
            break;
        case 0x15: // Fx15 — LD DT, Vx
            delay_timer = V[x];
            break;
        case 0x18: // Fx18 — LD ST, Vx
            sound_timer = V[x];
            break;
        case 0x1E: // Fx1E — ADD I, Vx
            I += V[x];
            break;
        case 0x29: // Fx29 — LD F, Vx (font sprite address)
            I = FONTSET_START + (V[x] & 0x0F) * 5;
            break;
        case 0x33: // Fx33 — BCD of Vx
            memory[I]     = V[x] / 100;
            memory[I + 1] = (V[x] / 10) % 10;
            memory[I + 2] = V[x] % 10;
            break;
        case 0x55: // Fx55 — LD [I], V0..Vx
            for (std::uint8_t i = 0; i <= x; ++i) {
                memory[I + i] = V[i];
            }
            break;
        case 0x65: // Fx65 — LD V0..Vx, [I]
            for (std::uint8_t i = 0; i <= x; ++i) {
                V[i] = memory[I + i];
            }
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
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
