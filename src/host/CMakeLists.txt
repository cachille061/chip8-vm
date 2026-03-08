add_executable(chip8
    main.cpp
)

target_link_libraries(chip8 PRIVATE chip8_core SDL2::SDL2 SDL2::SDL2main)
