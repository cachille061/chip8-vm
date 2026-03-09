#include <gtest/gtest.h>
#include "core/chip8.hpp"

using namespace chip8;

TEST(Chip8Init, ProgramCounterStartsAtRomStart)
{
    Chip8 vm;
    EXPECT_EQ(vm.pc, ROM_START);
}

TEST(Chip8Init, MemoryIsZeroOutsideFontset)
{
    Chip8 vm;
    for (std::size_t i = 0; i < FONTSET_START; ++i) {
        EXPECT_EQ(vm.memory[i], 0) << "at address " << i;
    }
    for (std::size_t i = FONTSET_START + 80; i < MEMORY_SIZE; ++i) {
        EXPECT_EQ(vm.memory[i], 0) << "at address " << i;
    }
}

TEST(Chip8Init, FontsetLoaded)
{
    Chip8 vm;
    // "0" sprite: F0 90 90 90 F0
    EXPECT_EQ(vm.memory[FONTSET_START + 0], 0xF0);
    EXPECT_EQ(vm.memory[FONTSET_START + 1], 0x90);
    EXPECT_EQ(vm.memory[FONTSET_START + 2], 0x90);
    EXPECT_EQ(vm.memory[FONTSET_START + 3], 0x90);
    EXPECT_EQ(vm.memory[FONTSET_START + 4], 0xF0);
    // "F" sprite: F0 80 F0 80 80
    EXPECT_EQ(vm.memory[FONTSET_START + 75], 0xF0);
    EXPECT_EQ(vm.memory[FONTSET_START + 76], 0x80);
    EXPECT_EQ(vm.memory[FONTSET_START + 77], 0xF0);
    EXPECT_EQ(vm.memory[FONTSET_START + 78], 0x80);
    EXPECT_EQ(vm.memory[FONTSET_START + 79], 0x80);
}

TEST(Chip8Init, RegistersAreZero)
{
    Chip8 vm;
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(vm.V[i], 0);
    }
    EXPECT_EQ(vm.I, 0);
    EXPECT_EQ(vm.sp, 0);
    EXPECT_EQ(vm.delay_timer, 0);
    EXPECT_EQ(vm.sound_timer, 0);
}

TEST(Chip8Init, DisplayIsBlank)
{
    Chip8 vm;
    for (auto pixel : vm.display) {
        EXPECT_EQ(pixel, 0);
    }
}

TEST(Chip8LoadRom, SmallRom)
{
    Chip8 vm;
    std::vector<std::uint8_t> rom = {0x12, 0x34, 0x56, 0x78};
    EXPECT_TRUE(vm.load_rom(rom));
    EXPECT_EQ(vm.memory[ROM_START + 0], 0x12);
    EXPECT_EQ(vm.memory[ROM_START + 1], 0x34);
    EXPECT_EQ(vm.memory[ROM_START + 2], 0x56);
    EXPECT_EQ(vm.memory[ROM_START + 3], 0x78);
}

TEST(Chip8LoadRom, TooLarge)
{
    Chip8 vm;
    std::vector<std::uint8_t> rom(MAX_ROM_SIZE + 1, 0xFF);
    EXPECT_FALSE(vm.load_rom(rom));
}

TEST(Chip8Timers, DelayTimerDecrements)
{
    Chip8 vm;
    vm.delay_timer = 5;
    vm.tick_timers();
    EXPECT_EQ(vm.delay_timer, 4);
}

TEST(Chip8Timers, SoundTimerSetsBeepFlag)
{
    Chip8 vm;
    vm.sound_timer = 3;
    vm.tick_timers();
    EXPECT_EQ(vm.sound_timer, 2);
    EXPECT_TRUE(vm.beep_flag);
}

TEST(Chip8Timers, BeepFlagClearsWhenSoundTimerZero)
{
    Chip8 vm;
    vm.sound_timer = 0;
    vm.beep_flag = true;
    vm.tick_timers();
    EXPECT_FALSE(vm.beep_flag);
}

TEST(Chip8Timers, TimersDontGoBelowZero)
{
    Chip8 vm;
    vm.delay_timer = 0;
    vm.sound_timer = 0;
    vm.tick_timers();
    EXPECT_EQ(vm.delay_timer, 0);
    EXPECT_EQ(vm.sound_timer, 0);
}
