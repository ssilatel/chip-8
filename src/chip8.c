#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.h"

SDL_Window*   window;
SDL_Renderer* renderer;

void initChip8();
int  loadROM(const char* filepath);

int main(int argc, char** argv)
{
    initChip8();
    loadROM("roms/ibm-logo.ch8");
    return 0;
}

void initChip8()
{
    memset(memory, 0, 4096);
    memset(display, 0, 64 * 32);
    memset(V, 0, 16);
    I = 0;
    PC = 0x200;  // 0x000 to 0x1FF is used by CHIP-8
    delayTimer = 0;
    soundTimer = 0;
    memset(stack, 0, 16);
    sp = 0;
    memset(keyboard, 0, 16);
    memcpy(memory + 0x50, fonts, 80 * sizeof(uint8_t));
}

int loadROM(const char* filepath)
{
    FILE* infile = fopen(filepath, "rb");
    if (infile == NULL)
    {
        fprintf(stderr, "Couldn't open ROM: %s\n", filepath);
        return 1;
    }

    fseek(infile, 0, SEEK_END);
    int size = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    fread(memory + 0x200, sizeof(uint16_t), size, infile);
    return 0;
}
