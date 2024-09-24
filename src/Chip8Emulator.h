#ifndef CHIP8EMULATOR_H
#define CHIP8EMULATOR_H

#include <SFML/Graphics.hpp>
#include <cstdint>

class Chip8Emulator
{
    public:
    Chip8Emulator();
    void initialize();
    void loadROM(const char* filename);
    void emulateCycle();
    void drawGraphics(sf::RenderWindow &window);
    void setKeys();

    private:
        uint8_t memory[4096]; // 4 kilobyte memory
        uint32_t gfx[64 * 32];     // 64 x 32 Display
        uint16_t PC;                     //Program Counter
        uint16_t I;                         // Index Register
        uint16_t stack[16];               //Stack
        uint8_t sp;                             //Stack pointer
        uint8_t delay_timer;               //Delay timer
        uint8_t sound_timer;            // sound timer
        uint8_t V[16];                // 16 8-bit registers [V0 - VF]
        uint8_t keypad[16]; //16 Keys for Keypad
        static const uint8_t fontset[80];
};

#endif