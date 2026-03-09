#include <gtest/gtest.h>
#include "chip8.hpp"

using namespace chip8;

// Helper: write a 2-byte opcode at the current pc and run one cycle.
static void exec(Chip8& vm, std::uint16_t opcode)
{
    vm.memory[vm.pc]     = (opcode >> 8) & 0xFF;
    vm.memory[vm.pc + 1] = opcode & 0xFF;
    vm.cycle();
}

// ============================================================
// Group A — Control flow & basics
// ============================================================

TEST(OpcodeGroupA, CLS_00E0)
{
    Chip8 vm;
    vm.display.fill(1);
    exec(vm, 0x00E0);
    for (auto p : vm.display) EXPECT_EQ(p, 0);
    EXPECT_TRUE(vm.draw_flag);
}

TEST(OpcodeGroupA, RET_00EE)
{
    Chip8 vm;
    vm.stack[0] = 0x400;
    vm.sp = 1;
    exec(vm, 0x00EE);
    EXPECT_EQ(vm.pc, 0x400);
    EXPECT_EQ(vm.sp, 0);
}

TEST(OpcodeGroupA, JP_1nnn)
{
    Chip8 vm;
    exec(vm, 0x1456);
    EXPECT_EQ(vm.pc, 0x456);
}

TEST(OpcodeGroupA, CALL_2nnn)
{
    Chip8 vm;
    std::uint16_t return_addr = vm.pc + 2; // pc advances past the opcode first
    exec(vm, 0x2456);
    EXPECT_EQ(vm.pc, 0x456);
    EXPECT_EQ(vm.sp, 1);
    EXPECT_EQ(vm.stack[0], return_addr);
}

TEST(OpcodeGroupA, SE_3xkk_Skip)
{
    Chip8 vm;
    vm.V[0x5] = 0x42;
    std::uint16_t start = vm.pc;
    exec(vm, 0x3542); // V5 == 0x42 → skip
    EXPECT_EQ(vm.pc, start + 4);
}

TEST(OpcodeGroupA, SE_3xkk_NoSkip)
{
    Chip8 vm;
    vm.V[0x5] = 0x41;
    std::uint16_t start = vm.pc;
    exec(vm, 0x3542); // V5 != 0x42 → no skip
    EXPECT_EQ(vm.pc, start + 2);
}

TEST(OpcodeGroupA, SNE_4xkk_Skip)
{
    Chip8 vm;
    vm.V[0x3] = 0x10;
    std::uint16_t start = vm.pc;
    exec(vm, 0x4320); // V3 != 0x20 → skip
    EXPECT_EQ(vm.pc, start + 4);
}

TEST(OpcodeGroupA, SNE_4xkk_NoSkip)
{
    Chip8 vm;
    vm.V[0x3] = 0x20;
    std::uint16_t start = vm.pc;
    exec(vm, 0x4320);
    EXPECT_EQ(vm.pc, start + 2);
}

TEST(OpcodeGroupA, SE_5xy0_Skip)
{
    Chip8 vm;
    vm.V[0x1] = 0xAB;
    vm.V[0x2] = 0xAB;
    std::uint16_t start = vm.pc;
    exec(vm, 0x5120);
    EXPECT_EQ(vm.pc, start + 4);
}

TEST(OpcodeGroupA, SE_5xy0_NoSkip)
{
    Chip8 vm;
    vm.V[0x1] = 0xAB;
    vm.V[0x2] = 0xCD;
    std::uint16_t start = vm.pc;
    exec(vm, 0x5120);
    EXPECT_EQ(vm.pc, start + 2);
}

TEST(OpcodeGroupA, SNE_9xy0_Skip)
{
    Chip8 vm;
    vm.V[0x1] = 0xAB;
    vm.V[0x2] = 0xCD;
    std::uint16_t start = vm.pc;
    exec(vm, 0x9120);
    EXPECT_EQ(vm.pc, start + 4);
}

TEST(OpcodeGroupA, SNE_9xy0_NoSkip)
{
    Chip8 vm;
    vm.V[0x1] = 0xAB;
    vm.V[0x2] = 0xAB;
    std::uint16_t start = vm.pc;
    exec(vm, 0x9120);
    EXPECT_EQ(vm.pc, start + 2);
}

TEST(OpcodeGroupA, LD_6xkk)
{
    Chip8 vm;
    exec(vm, 0x6A42);
    EXPECT_EQ(vm.V[0xA], 0x42);
}

TEST(OpcodeGroupA, ADD_7xkk)
{
    Chip8 vm;
    vm.V[0x3] = 0x10;
    exec(vm, 0x7305);
    EXPECT_EQ(vm.V[0x3], 0x15);
}

TEST(OpcodeGroupA, ADD_7xkk_Wraps)
{
    Chip8 vm;
    vm.V[0x0] = 0xFF;
    exec(vm, 0x7002);
    EXPECT_EQ(vm.V[0x0], 0x01); // wraps without touching VF
}

TEST(OpcodeGroupA, LD_Annn)
{
    Chip8 vm;
    exec(vm, 0xA123);
    EXPECT_EQ(vm.I, 0x123);
}

TEST(OpcodeGroupA, JP_Bnnn)
{
    Chip8 vm;
    vm.V[0] = 0x10;
    exec(vm, 0xB300);
    EXPECT_EQ(vm.pc, 0x310);
}

// ============================================================
// Group B — Arithmetic & logic (8xy_)
// ============================================================

TEST(OpcodeGroupB, LD_8xy0)
{
    Chip8 vm;
    vm.V[0x1] = 0xAB;
    exec(vm, 0x8010); // V0 = V1
    EXPECT_EQ(vm.V[0x0], 0xAB);
}

TEST(OpcodeGroupB, OR_8xy1)
{
    Chip8 vm;
    vm.V[0x0] = 0x0F;
    vm.V[0x1] = 0xF0;
    exec(vm, 0x8011);
    EXPECT_EQ(vm.V[0x0], 0xFF);
    EXPECT_EQ(vm.V[0xF], 0); // VF reset
}

TEST(OpcodeGroupB, AND_8xy2)
{
    Chip8 vm;
    vm.V[0x0] = 0x0F;
    vm.V[0x1] = 0xF5;
    exec(vm, 0x8012);
    EXPECT_EQ(vm.V[0x0], 0x05);
    EXPECT_EQ(vm.V[0xF], 0);
}

TEST(OpcodeGroupB, XOR_8xy3)
{
    Chip8 vm;
    vm.V[0x0] = 0xFF;
    vm.V[0x1] = 0x0F;
    exec(vm, 0x8013);
    EXPECT_EQ(vm.V[0x0], 0xF0);
    EXPECT_EQ(vm.V[0xF], 0);
}

TEST(OpcodeGroupB, ADD_8xy4_NoCarry)
{
    Chip8 vm;
    vm.V[0x0] = 0x10;
    vm.V[0x1] = 0x20;
    exec(vm, 0x8014);
    EXPECT_EQ(vm.V[0x0], 0x30);
    EXPECT_EQ(vm.V[0xF], 0);
}

TEST(OpcodeGroupB, ADD_8xy4_Carry)
{
    Chip8 vm;
    vm.V[0x0] = 0xFF;
    vm.V[0x1] = 0x02;
    exec(vm, 0x8014);
    EXPECT_EQ(vm.V[0x0], 0x01);
    EXPECT_EQ(vm.V[0xF], 1);
}

TEST(OpcodeGroupB, SUB_8xy5_NoBorrow)
{
    Chip8 vm;
    vm.V[0x0] = 0x30;
    vm.V[0x1] = 0x10;
    exec(vm, 0x8015);
    EXPECT_EQ(vm.V[0x0], 0x20);
    EXPECT_EQ(vm.V[0xF], 1); // Vx >= Vy → VF = 1
}

TEST(OpcodeGroupB, SUB_8xy5_Borrow)
{
    Chip8 vm;
    vm.V[0x0] = 0x10;
    vm.V[0x1] = 0x30;
    exec(vm, 0x8015);
    EXPECT_EQ(vm.V[0x0], static_cast<std::uint8_t>(0x10 - 0x30));
    EXPECT_EQ(vm.V[0xF], 0);
}

TEST(OpcodeGroupB, SHR_8xy6)
{
    Chip8 vm;
    vm.V[0x1] = 0b00000111; // Vy = 7
    exec(vm, 0x8016);       // Vx = Vy >> 1
    EXPECT_EQ(vm.V[0x0], 0b00000011);
    EXPECT_EQ(vm.V[0xF], 1); // LSB was 1
}

TEST(OpcodeGroupB, SHR_8xy6_ZeroLSB)
{
    Chip8 vm;
    vm.V[0x1] = 0b00000110;
    exec(vm, 0x8016);
    EXPECT_EQ(vm.V[0x0], 0b00000011);
    EXPECT_EQ(vm.V[0xF], 0); // LSB was 0
}

TEST(OpcodeGroupB, SUBN_8xy7_NoBorrow)
{
    Chip8 vm;
    vm.V[0x0] = 0x10;
    vm.V[0x1] = 0x30;
    exec(vm, 0x8017);
    EXPECT_EQ(vm.V[0x0], 0x20);
    EXPECT_EQ(vm.V[0xF], 1);
}

TEST(OpcodeGroupB, SUBN_8xy7_Borrow)
{
    Chip8 vm;
    vm.V[0x0] = 0x30;
    vm.V[0x1] = 0x10;
    exec(vm, 0x8017);
    EXPECT_EQ(vm.V[0x0], static_cast<std::uint8_t>(0x10 - 0x30));
    EXPECT_EQ(vm.V[0xF], 0);
}

TEST(OpcodeGroupB, SHL_8xyE)
{
    Chip8 vm;
    vm.V[0x1] = 0b10000001;
    exec(vm, 0x801E);
    EXPECT_EQ(vm.V[0x0], 0b00000010);
    EXPECT_EQ(vm.V[0xF], 1); // MSB was 1
}

TEST(OpcodeGroupB, SHL_8xyE_ZeroMSB)
{
    Chip8 vm;
    vm.V[0x1] = 0b01000001;
    exec(vm, 0x801E);
    EXPECT_EQ(vm.V[0x0], 0b10000010);
    EXPECT_EQ(vm.V[0xF], 0);
}

// ============================================================
// Group C — Timers, memory, BCD, random
// ============================================================

TEST(OpcodeGroupC, LD_Fx07)
{
    Chip8 vm;
    vm.delay_timer = 42;
    exec(vm, 0xF007); // V0 = DT
    EXPECT_EQ(vm.V[0x0], 42);
}

TEST(OpcodeGroupC, LD_Fx15)
{
    Chip8 vm;
    vm.V[0x3] = 30;
    exec(vm, 0xF315); // DT = V3
    EXPECT_EQ(vm.delay_timer, 30);
}

TEST(OpcodeGroupC, LD_Fx18)
{
    Chip8 vm;
    vm.V[0x5] = 15;
    exec(vm, 0xF518); // ST = V5
    EXPECT_EQ(vm.sound_timer, 15);
}

TEST(OpcodeGroupC, ADD_Fx1E)
{
    Chip8 vm;
    vm.I = 0x100;
    vm.V[0x2] = 0x10;
    exec(vm, 0xF21E);
    EXPECT_EQ(vm.I, 0x110);
}

TEST(OpcodeGroupC, LD_Fx29)
{
    Chip8 vm;
    vm.V[0x0] = 0xA; // character 'A'
    exec(vm, 0xF029);
    EXPECT_EQ(vm.I, FONTSET_START + 0xA * 5);
}

TEST(OpcodeGroupC, BCD_Fx33)
{
    Chip8 vm;
    vm.V[0x0] = 254;
    vm.I = 0x300;
    exec(vm, 0xF033);
    EXPECT_EQ(vm.memory[0x300], 2); // hundreds
    EXPECT_EQ(vm.memory[0x301], 5); // tens
    EXPECT_EQ(vm.memory[0x302], 4); // ones
}

TEST(OpcodeGroupC, BCD_Fx33_Zero)
{
    Chip8 vm;
    vm.V[0x0] = 0;
    vm.I = 0x300;
    exec(vm, 0xF033);
    EXPECT_EQ(vm.memory[0x300], 0);
    EXPECT_EQ(vm.memory[0x301], 0);
    EXPECT_EQ(vm.memory[0x302], 0);
}

TEST(OpcodeGroupC, LD_Fx55_Store)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.V[0] = 0xAA;
    vm.V[1] = 0xBB;
    vm.V[2] = 0xCC;
    exec(vm, 0xF255); // store V0–V2
    EXPECT_EQ(vm.memory[0x300], 0xAA);
    EXPECT_EQ(vm.memory[0x301], 0xBB);
    EXPECT_EQ(vm.memory[0x302], 0xCC);
}

TEST(OpcodeGroupC, LD_Fx65_Load)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.memory[0x300] = 0x11;
    vm.memory[0x301] = 0x22;
    vm.memory[0x302] = 0x33;
    exec(vm, 0xF265); // load V0–V2
    EXPECT_EQ(vm.V[0], 0x11);
    EXPECT_EQ(vm.V[1], 0x22);
    EXPECT_EQ(vm.V[2], 0x33);
}

TEST(OpcodeGroupC, RND_Cxkk_Masked)
{
    Chip8 vm;
    // Run RND multiple times — result should always be masked by kk
    for (int i = 0; i < 100; ++i) {
        vm.pc = ROM_START;
        exec(vm, 0xC00F); // V0 = random & 0x0F
        EXPECT_LE(vm.V[0], 0x0F);
    }
}

// ============================================================
// Group D — Display (Dxyn)
// ============================================================

TEST(OpcodeGroupD, DRW_SinglePixel)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.memory[0x300] = 0x80; // top-left pixel of 1-row sprite
    vm.V[0x0] = 0; // x
    vm.V[0x1] = 0; // y
    exec(vm, 0xD011); // draw 1-row sprite at (0,0)
    EXPECT_EQ(vm.display[0], 1);
    EXPECT_EQ(vm.V[0xF], 0); // no collision
    EXPECT_TRUE(vm.draw_flag);
}

TEST(OpcodeGroupD, DRW_Collision)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.memory[0x300] = 0x80;
    vm.V[0x0] = 0;
    vm.V[0x1] = 0;
    vm.display[0] = 1; // pre-set pixel
    exec(vm, 0xD011);
    EXPECT_EQ(vm.display[0], 0); // XOR → off
    EXPECT_EQ(vm.V[0xF], 1);    // collision detected
}

TEST(OpcodeGroupD, DRW_XORToggle)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.memory[0x300] = 0x80;
    vm.V[0x0] = 0;
    vm.V[0x1] = 0;

    // Draw once → pixel on
    exec(vm, 0xD011);
    EXPECT_EQ(vm.display[0], 1);

    // Draw again at same location → pixel off (XOR)
    vm.pc = ROM_START;
    exec(vm, 0xD011);
    EXPECT_EQ(vm.display[0], 0);
    EXPECT_EQ(vm.V[0xF], 1); // collision on second draw
}

TEST(OpcodeGroupD, DRW_Clipping)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.memory[0x300] = 0xFF; // full 8-pixel row
    vm.V[0x0] = 60; // x = 60, so only 4 pixels fit (60..63)
    vm.V[0x1] = 0;
    exec(vm, 0xD011);
    EXPECT_EQ(vm.display[60], 1);
    EXPECT_EQ(vm.display[61], 1);
    EXPECT_EQ(vm.display[62], 1);
    EXPECT_EQ(vm.display[63], 1);
}

TEST(OpcodeGroupD, DRW_MultiRow)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.memory[0x300] = 0x80; // row 0: leftmost pixel
    vm.memory[0x301] = 0x80; // row 1: leftmost pixel
    vm.V[0x0] = 0;
    vm.V[0x1] = 0;
    exec(vm, 0xD012); // 2-row sprite
    EXPECT_EQ(vm.display[0], 1);                    // row 0, col 0
    EXPECT_EQ(vm.display[DISPLAY_WIDTH], 1);         // row 1, col 0
}

TEST(OpcodeGroupD, DRW_WrapsCoordinates)
{
    Chip8 vm;
    vm.I = 0x300;
    vm.memory[0x300] = 0x80;
    vm.V[0x0] = 64; // wraps to x=0
    vm.V[0x1] = 32; // wraps to y=0
    exec(vm, 0xD011);
    EXPECT_EQ(vm.display[0], 1);
}

// ============================================================
// Group E — Keypad
// ============================================================

TEST(OpcodeGroupE, SKP_Ex9E_Pressed)
{
    Chip8 vm;
    vm.V[0x0] = 0x5;
    vm.keypad[0x5] = true;
    std::uint16_t start = vm.pc;
    exec(vm, 0xE09E);
    EXPECT_EQ(vm.pc, start + 4);
}

TEST(OpcodeGroupE, SKP_Ex9E_NotPressed)
{
    Chip8 vm;
    vm.V[0x0] = 0x5;
    vm.keypad[0x5] = false;
    std::uint16_t start = vm.pc;
    exec(vm, 0xE09E);
    EXPECT_EQ(vm.pc, start + 2);
}

TEST(OpcodeGroupE, SKNP_ExA1_NotPressed)
{
    Chip8 vm;
    vm.V[0x0] = 0x5;
    vm.keypad[0x5] = false;
    std::uint16_t start = vm.pc;
    exec(vm, 0xE0A1);
    EXPECT_EQ(vm.pc, start + 4);
}

TEST(OpcodeGroupE, SKNP_ExA1_Pressed)
{
    Chip8 vm;
    vm.V[0x0] = 0x5;
    vm.keypad[0x5] = true;
    std::uint16_t start = vm.pc;
    exec(vm, 0xE0A1);
    EXPECT_EQ(vm.pc, start + 2);
}

TEST(OpcodeGroupE, WaitForKey_Fx0A)
{
    Chip8 vm;
    exec(vm, 0xF20A); // LD V2, K — wait for key
    EXPECT_TRUE(vm.waiting_for_key);
    EXPECT_EQ(vm.key_register, 2);

    // Cycle with no key → still waiting
    vm.cycle();
    EXPECT_TRUE(vm.waiting_for_key);

    // Press key 7
    vm.keypad[0x7] = true;
    vm.cycle();
    EXPECT_FALSE(vm.waiting_for_key);
    EXPECT_EQ(vm.V[2], 0x7);
}

// ============================================================
// Integration — multi-instruction sequences
// ============================================================

TEST(Integration, CallAndReturn)
{
    Chip8 vm;
    // At 0x200: CALL 0x300
    vm.memory[0x200] = 0x23;
    vm.memory[0x201] = 0x00;
    // At 0x300: LD V0, 0x42
    vm.memory[0x300] = 0x60;
    vm.memory[0x301] = 0x42;
    // At 0x302: RET
    vm.memory[0x302] = 0x00;
    vm.memory[0x303] = 0xEE;

    vm.cycle(); // CALL 0x300
    EXPECT_EQ(vm.pc, 0x300);
    vm.cycle(); // LD V0, 0x42
    EXPECT_EQ(vm.V[0], 0x42);
    vm.cycle(); // RET
    EXPECT_EQ(vm.pc, 0x202);
}

TEST(Integration, ConditionalSkip)
{
    Chip8 vm;
    // 0x200: LD V0, 5   → 6005
    // 0x202: SE V0, 5   → 3005  (should skip next)
    // 0x204: LD V1, 0xFF (skipped)
    // 0x206: LD V1, 0x01
    vm.memory[0x200] = 0x60; vm.memory[0x201] = 0x05;
    vm.memory[0x202] = 0x30; vm.memory[0x203] = 0x05;
    vm.memory[0x204] = 0x61; vm.memory[0x205] = 0xFF;
    vm.memory[0x206] = 0x61; vm.memory[0x207] = 0x01;

    vm.cycle(); // LD V0, 5
    vm.cycle(); // SE V0, 5 → skip
    vm.cycle(); // LD V1, 0x01
    EXPECT_EQ(vm.V[0], 5);
    EXPECT_EQ(vm.V[1], 0x01); // 0xFF was skipped
}
