#include "Chip8Emulator.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdint>
#include <fstream>
#include <filesystem>

Chip8Emulator::Chip8Emulator()
{
    initialize();
}

void Chip8Emulator::initialize() 
{
    PC = 0x200;
    I = 0;
    std::fill(std::begin(stack), std::end(stack), 0);
    delay_timer = 0;
    sound_timer = 0;
    std::fill(std::begin(V), std::end(V), 0);

    std::fill(std::begin(gfx), std::end(gfx), 0);
}

const uint8_t Chip8Emulator::fontset[80] = 
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8Emulator::loadROM(const char* filename)
{
    // - NEED TO ADD ERROR HANDLING LATER 

    std::ifstream romFile(filename, std::ios::binary); // Load ROM in binary

    if (!std::filesystem::exists(filename)) 
    {
        std::cerr << "File does not exist: " << filename << std::endl;
        return;
    }

    if(!romFile)
    {
        std::cerr << "Failed to Open Rom";
        return;
    }

    romFile.read(reinterpret_cast<char*>(&memory[PC]), 4096 - PC); // Read file into memory starting address 0x200
    romFile.close();

}

// Dont even ask about this function those cases killed me
void Chip8Emulator::emulateCycle()
{
    //FETCH
    //Retrive instruction from memory reads current byte and byte following
    //Shift first byte left by 8 bits combine with OR operator creating one 16 bit instruction
    //Increment program counter
    uint16_t instruction = memory[PC] << 8 | memory[PC + 1];
    PC += 2;


    switch(instruction & 0xF000)
    {
        case 0x0000:
        {
            //Clear Display
            if(instruction == 0x00E0)
            {
                std::fill(std::begin(gfx), std::end(gfx), 0);
            }
            //Return from subroutine
            else if(instruction == 0x00EE)
            {
                PC = stack[--sp];
            }
            break;
        }

    case 0x1000: // 1NNN - Jump to address NNN
        {
            uint16_t address = instruction & 0x0FFF;
            PC = address;
            break;
        }


    case 0x2000: // 2NNN - Call subroutine at NNN
        {
            uint16_t address = instruction & 0x0FFF;
            stack[sp++] = PC; 
            PC = address;
            break;
        }


    case 0x3000: // 3XKK - Skip next instruction if Vx == KK
        {
            uint8_t Vx = (instruction & 0x0F00) >> 8; 
            uint8_t byte = instruction & 0x00FF; 
            if (V[Vx] == byte) {
                PC += 2; 
            }
            break;
        }


    case 0x4000: //4XKK - Skip Next instruction if Vx != KK
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t byte = instruction & 0x00FF;

        if (V[Vx] != byte)
        {
            PC +=2;
        }
        break;
    }


    case 0x5000: //5xy0 Skip next instruction if Vx == Vy
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;

        if (V[Vx] == V[Vy])
        {
            PC += 2;
        }
        break;
    }


    case 0x6000: //6xkk Vx = Byte
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t byte = instruction & 0x00FFu;

        V[Vx] = byte;
        break;
    }


    case 0x7000: //7xkk ADD Vx to byte
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t byte = instruction & 0x00FFu;
        
        V[Vx] += byte;
       break;
   }


    case 0x8000: //Set Vx = Vy
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;
        V[Vx] = V[Vy];
        break;
    }


    case 0x8001: //Set Vx = Vx OR vY
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;
        V[Vx] |= V[Vy];
        break;
    }


    case 0x8002: //Set Vx = Vx AND Vy
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;
        V[Vx] &= V[Vy];
        break;
    }


    case 0x8003: //Set Vx = Vx XOR Vy
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;
        V[Vx] ^= V[Vy];
        break;
    }


    case 0x8004: // Add Vx and Vy if result > 8bits VF = 1 otherwise 0. Only lowest 8 bits are kept
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;
        
        uint16_t sum = V[Vx] + V[Vy];

        if (sum > 255)
        {
            V[0xF] = 1;
        }
        else
        {
            V[0xF] = 0;
        }

        V[Vx] = sum & 0xFF;
        break;
    }


    case 0x8005: //If Vx > Vy then VF is set to 1 otherwise 0. Then Vy is subtracted from Vx and results are stored in Vx
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;

        if(V[Vx] > V[Vy])
        {
            V[0xF] = 1;
        }
        else
        {
            V[0xF] = 0;
        }

        V[Vx] -= V[Vy];
        break;
    }


    case 0x8006: //If least-sig bit of Vx is 1 then VF = 1 otherwise 0. Vx then divided by 2 by rightshift
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;

        V[0xF] = (V[Vx] & 0x1u);

        V[Vx] >>= 1;
        break;
    }


    case 0x8007: // Set Vx = Vy - Vx. If Vy > Vx then VF = 1 otherwise 0, Vx - Vy results in Vx
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;

        if(V[Vy] > V[Vx])
        {
            V[0xF] = 1;
        }   
        else
        {
            V[0xF] = 0;
        }
        V[Vx] = V[Vy] - V[Vx];
        break;
    }


    case 0x800E: // If most sig bit of Vx is 1 then VF is 1 otherwise 0 then Vx * 2
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;

        V[0xF] = (V[Vx] & 0x80) >> 7;
        V[Vx] <<= 1;
      break;
    }
  

    case 0x9000: // Skip next instruction if Vx != Vy
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t Vy = (instruction & 0x00F0) >> 4;

        if(V[Vx] != V[Vy])
        {
            PC += 2;
        }
        break;
    }


    case 0xA000: // Set Vx = Vy - Vx. If Vy > Vx then VF = 1 otherwise 0, Vx - Vy results in Vx
    {
        uint16_t address = instruction & 0x0FFF;

        I = address; //Index register = Address
    }
    break;

    case 0xB000: // Jump to location nnn + V0
    {
        uint16_t address = instruction & 0x0FFF;

        PC = address + V[0]; 
    }
    break;

    case 0xC000: // Vx = random byte and Byte(kk)
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8u;
        uint8_t byte = instruction & 0x00FF;
        uint8_t randomByte = rand() % 256;

        V[Vx] = randomByte & byte;
        break;
    }


    case 0xD000: // Draw sprite at location Vx, Vn with height N
    // - LOT OF PROBLEM WITH THIS ONE STILL DONT KNOW IF IT WILL WORK WELL
    // - DONT REALLY UNDERSTAND IT LEARN LATER
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8u;
        uint8_t Vy = (instruction & 0x00F0) >> 4u;
        uint8_t height = instruction & 0x00F;

        uint8_t xPos = V[Vx] % 64;
        uint8_t yPos = V[Vy] % 32;

        V[0xF] = 0;

        for(unsigned int row = 0; row < height; ++row)
        {
            uint8_t spriteByte = memory[I + row];

            for (unsigned int col = 0; col < 8; ++col)
            {
                uint8_t spritePixel = spriteByte & (0x80 >> col);
                uint32_t* screenPixel = &gfx[(yPos + row) * 64 + (xPos + col)];

                if (spritePixel)
                {
                    if(*screenPixel == 0xFFFFFFFF)
                    {
                        V[0xF] = 1;
                    }

                    *screenPixel ^= 0xFFFFFFFF;
                }
            }
        }
        break;
    }

    case 0xE000: // Skip the NEXT instruction if key with value of Vx is pressed
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        uint8_t key = V[Vx];

        switch (instruction & 0x00FF)
        {
            case 0x9E:
                if(keypad[key])
                {
                    PC += 2;
                }
                break;

            case 0xA1:
                if(!keypad[key])
                {
                    PC +=2;
                }
                break;
        }
    break;
    }


    case 0xF000:
    {
        uint8_t Vx = (instruction & 0x0F00) >> 8;
        switch (instruction & 0x00FF) 
        {
            case 0x07:
            {
                V[Vx] = delay_timer;
                break;
            }

            case 0x0A:
            {
                if (keypad[0])
                {
                    V[Vx] = 0;
                }
                else if (keypad[1])
                {
                    V[Vx] = 1;
                }
                else if (keypad[2])
                {
                    V[Vx] = 2;
                }
                else if (keypad[3])
                {
                    V[Vx] = 3;
                }
                else if (keypad[4])
                {
                    V[Vx] = 4;
                }
                else if (keypad[5])
                {
                    V[Vx] = 5;
                }
                else if (keypad[6])
                {
                    V[Vx] = 6;
                }
                else if (keypad[7])
                {
                    V[Vx] = 7;
                }
                else if (keypad[8])
                {
                    V[Vx] = 8;
                }
                else if (keypad[9])
                {
                    V[Vx] = 9;
                }
                else if (keypad[10])
                {
                    V[Vx] = 10;
                }
                else if (keypad[11])
                {
                    V[Vx] = 11;
                }
                else if (keypad[12])
                {
                    V[Vx] = 12;
                }
                else if (keypad[13])
                {
                    V[Vx] = 13;
                }
                else if (keypad[14])
                {
                    V[Vx] = 14;
                }
                else if (keypad[15])
                {
                    V[Vx] = 15;
                }
                else
                {
                    PC -= 2;
                }
                break;
            }

            case 0x15:
            {
                delay_timer = V[Vx];
                break;
            } 

            case 0x18:
            {
                sound_timer = V[Vx];
                break;
            }

            case 0x1E:
            {
                I += V[Vx];
                break;
            }

            case 0x29:
            {
                I = V[Vx] * 5;
                break;
            }

            case 0x33:
            {
                memory[I] = V[Vx] / 100;
                memory[I + 1] = (V[Vx] / 10) % 10;
                memory[I + 2] = V[Vx] % 10;
                break;
            }

            case 0x55:
            {
                for(uint8_t reg=0; reg <= Vx; ++reg)
                {
                    memory[I + reg] = V[reg];
                }
                break;
            }

            case 0x65:
            {
                for (uint8_t reg =0; reg <= Vx; ++reg)
                {
                    V[reg] = memory[I + reg];
                }
                break;;
            }

            default:
            {
                break;
            }
        }
        break;
    }

    //Decrement delay timer
    if (delay_timer > 0)
    {
        --delay_timer;
    }

    //Decrememnt sound timer
    if (sound_timer > 0)
    {
        --sound_timer;
    }
}
}


void Chip8Emulator::drawGraphics(sf::RenderWindow& window)
{
    for (int y = 0; y < 32;  ++y) // loop over height
    {
        for(int x = 0; x < 64; ++x) //loop over width
        {
            if(gfx[y * 64 + x]) //If current pixel is on display that pixel
            {
                sf::RectangleShape pixel(sf::Vector2f(10,10));
                pixel.setPosition(x * 10, y * 10);
                pixel.setFillColor(sf::Color::White);
                window.draw(pixel);
            }
        }
    }
}


int main()
{
    Chip8Emulator emulator;
    emulator.initialize();
    emulator.loadROM("../romTest/test_opcode.ch8");

    sf::RenderWindow window(sf::VideoMode(640, 320), "CHIP-8 Emulator");
    
    while (window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        emulator.emulateCycle();
        window.clear();
        emulator.drawGraphics(window);
        window.display();
    }

    return 0;
}