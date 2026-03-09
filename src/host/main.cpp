#include "audio.hpp"
#include "display.hpp"
#include "input.hpp"
#include "core/chip8.hpp"

#include <SDL.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int DEFAULT_SCALE       = 10;
constexpr int CYCLES_PER_FRAME    = 12;
constexpr int TARGET_FPS          = 60;
constexpr int FRAME_DELAY_MS      = 1000 / TARGET_FPS;

struct Config {
    std::string rom_path;
    int scale  = DEFAULT_SCALE;
    int cycles = CYCLES_PER_FRAME;
};

bool parse_args(int argc, char* argv[], Config& cfg)
{
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            cfg.scale = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--speed") == 0 && i + 1 < argc) {
            cfg.cycles = std::atoi(argv[++i]);
        } else if (argv[i][0] != '-') {
            cfg.rom_path = argv[i];
        } else {
            std::cerr << "Unknown option: " << argv[i] << "\n";
            return false;
        }
    }
    if (cfg.rom_path.empty()) {
        std::cerr << "Usage: chip8 [--scale N] [--speed N] <rom>\n";
        return false;
    }
    return true;
}

std::vector<std::uint8_t> load_file(const std::string& path)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f)
        return {};
    auto size = f.tellg();
    f.seekg(0);
    std::vector<std::uint8_t> buf(static_cast<std::size_t>(size));
    f.read(reinterpret_cast<char*>(buf.data()), size);
    return buf;
}

} // anonymous namespace

int main(int argc, char* argv[])
{
    Config cfg;
    if (!parse_args(argc, argv, cfg))
        return 1;

    auto rom = load_file(cfg.rom_path);
    if (rom.empty()) {
        std::cerr << "Failed to open ROM: " << cfg.rom_path << "\n";
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    try {
        chip8::Display display(cfg.scale);
        chip8::Audio   audio;
        chip8::Input   input;
        chip8::Chip8   vm;

        if (!vm.load_rom(rom)) {
            std::cerr << "ROM too large (" << rom.size() << " bytes).\n";
            SDL_Quit();
            return 1;
        }

        bool running = true;
        bool paused  = false;
        int  frames  = 0;
        Uint32 fps_timer = SDL_GetTicks();

        while (running) {
            Uint32 frame_start = SDL_GetTicks();

            // 1. Poll host input
            switch (input.poll(vm.keypad)) {
                case chip8::Action::quit:
                    running = false;
                    break;
                case chip8::Action::toggle_pause:
                    paused = !paused;
                    break;
                case chip8::Action::reset:
                    vm = chip8::Chip8{};
                    vm.load_rom(rom);
                    paused = false;
                    break;
                case chip8::Action::none:
                    break;
            }

            if (!paused) {
                // 2. Run N instruction cycles per frame
                for (int i = 0; i < cfg.cycles; ++i)
                    vm.cycle();

                // 3. Tick timers at 60 Hz
                vm.tick_timers();
            }

            // 4. Render when the interpreter set draw_flag
            if (vm.draw_flag) {
                display.render(vm.display);
                vm.draw_flag = false;
            }

            // 5. Audio: beep while sound timer > 0 (silence when paused)
            if (vm.beep_flag && !paused)
                audio.beep();
            else
                audio.silence();

            // 6. Update title with FPS every second
            ++frames;
            Uint32 now = SDL_GetTicks();
            if (now - fps_timer >= 1000) {
                std::string title = "CHIP-8";
                if (paused)
                    title += " [PAUSED]";
                else
                    title += " | " + std::to_string(frames) + " FPS";
                display.set_title(title.c_str());
                frames = 0;
                fps_timer = now;
            }

            // 7. Cap at ~60 FPS
            Uint32 elapsed = SDL_GetTicks() - frame_start;
            if (elapsed < static_cast<Uint32>(FRAME_DELAY_MS))
                SDL_Delay(static_cast<Uint32>(FRAME_DELAY_MS) - elapsed);
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Quit();
    return 0;
}
