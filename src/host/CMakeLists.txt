add_executable(chip8
    main.cpp
    display.cpp
    input.cpp
    audio.cpp
)

target_link_libraries(chip8 PRIVATE chip8_core SDL2::SDL2main SDL2::SDL2)

# Keep console output (SDL2 adds -mwindows which hides it).
set(SDL2_NO_MWINDOWS TRUE)
target_link_options(chip8 PRIVATE -mconsole)
