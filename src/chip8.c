#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.h"

SDL_Window*   window;
SDL_Renderer* renderer;
SDL_Texture*  screen;

void initChip8();
int  loadROM(const char* filepath);
void draw();
void execute();
void cleanupSDL();

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, 64, 32);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    initChip8();
    if (loadROM("roms/ibm-logo.ch8") != 0)
    {
        cleanupSDL();
        return 1;
    }

    uint8_t running = 1;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            running = 0;
                            break;
                        case SDLK_1:
                            keyboard[0x1] = 1;
                            break;
                        case SDLK_2:
                            keyboard[0x2] = 1;
                            break;
                        case SDLK_3:
                            keyboard[0x3] = 1;
                            break;
                        case SDLK_4:
                            keyboard[0xC] = 1;
                            break;
                        case SDLK_q:
                            keyboard[0x4] = 1;
                            break;
                        case SDLK_w:
                            keyboard[0x5] = 1;
                            break;
                        case SDLK_e:
                            keyboard[0x6] = 1;
                            break;
                        case SDLK_r:
                            keyboard[0xD] = 1;
                            break;
                        case SDLK_a:
                            keyboard[0x7] = 1;
                            break;
                        case SDLK_s:
                            keyboard[0x8] = 1;
                            break;
                        case SDLK_d:
                            keyboard[0x9] = 1;
                            break;
                        case SDLK_f:
                            keyboard[0xE] = 1;
                            break;
                        case SDLK_z:
                            keyboard[0xA] = 1;
                            break;
                        case SDLK_x:
                            keyboard[0x0] = 1;
                            break;
                        case SDLK_c:
                            keyboard[0xB] = 1;
                            break;
                        case SDLK_v:
                            keyboard[0xF] = 1;
                            break;
                    }
                break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_1:
                            keyboard[0x1] = 0;
                            break;
                        case SDLK_2:
                            keyboard[0x2] = 0;
                            break;
                        case SDLK_3:
                            keyboard[0x3] = 0;
                            break;
                        case SDLK_4:
                            keyboard[0xC] = 0;
                            break;
                        case SDLK_q:
                            keyboard[0x4] = 0;
                            break;
                        case SDLK_w:
                            keyboard[0x5] = 0;
                            break;
                        case SDLK_e:
                            keyboard[0x6] = 0;
                            break;
                        case SDLK_r:
                            keyboard[0xD] = 0;
                            break;
                        case SDLK_a:
                            keyboard[0x7] = 0;
                            break;
                        case SDLK_s:
                            keyboard[0x8] = 0;
                            break;
                        case SDLK_d:
                            keyboard[0x9] = 0;
                            break;
                        case SDLK_f:
                            keyboard[0xE] = 0;
                            break;
                        case SDLK_z:
                            keyboard[0xA] = 0;
                            break;
                        case SDLK_x:
                            keyboard[0x0] = 0;
                            break;
                        case SDLK_c:
                            keyboard[0xB] = 0;
                            break;
                        case SDLK_v:
                            keyboard[0xF] = 0;
                            break;
                    }
                break;
            }
        }

        if (delayTimer > 0)
        {
            delayTimer--;
        }

        execute();
        draw();
    }
    cleanupSDL();
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

void draw()
{
    uint32_t     pixels[64 * 32];
    unsigned int x, y;

    if (drawFlag)
    {
        memset(pixels, 0, (64 * 32) * 4);
        for (x = 0; x < 64; x++)
        {
            for (y = 0; y < 32; y++)
            {
                if (display[x + (y * 64)] == 1)
                {
                    pixels[x + (y * 64)] = UINT32_MAX;
                }
            }
        }

        SDL_UpdateTexture(screen, NULL, pixels, 64 * sizeof(uint32_t));

        SDL_Rect pos;
        pos.x = 0;
        pos.y = 0;
        pos.w = 64;
        pos.h = 32;
        SDL_RenderCopy(renderer, screen, NULL, &pos);
        SDL_RenderPresent(renderer);
    }
    drawFlag = 0;
}

void execute()
{
    uint8_t  X, Y, kk, n;
    uint16_t nnn;

    // Fetch
    opcode = memory[PC] << 8 | memory[PC + 1];
    PC += 2;

    // Decoding
    X = (opcode & 0x0F00) >> 8;
    Y = (opcode & 0x00F0) >> 4;
    n = (opcode & 0x000F);
    kk = (opcode & 0x00FF);
    nnn = (opcode & 0x0FFF);
    printf("Opcode: %x\n", opcode);
    printf("Program Counter: %x\n", PC);
    printf("I: %x\n", I);

    switch (opcode & 0xF000)
    {
        case 0x0000:
            switch (opcode & 0x00FF)
            {
                case 0x00E0:
                    memset(display, 0, 64 * 32);
                    break;
            }
            break;
        case 0x1000:
            PC = nnn;
            break;
        case 0x2000:
            break;
        case 0x3000:
            break;
        case 0x4000:
            break;
        case 0x5000:
            break;
        case 0x6000:
            V[X] = kk;
            break;
        case 0x7000:
            V[X] += kk;
            break;
        case 0x8000:
            break;
        case 0x9000:
            break;
        case 0xA000:
            I = nnn;
            break;
        case 0xB000:
            break;
        case 0xC000:
            break;
        case 0xD000:
            uint8_t x = V[X] % 64;
            uint8_t y = V[Y] % 32;
            uint8_t pixel;

            V[0xF] = 0;
            for (int i = 0; i < n; i++)
            {
                pixel = memory[I + i];
                for (int j = 0; j < 8; j++)
                {
                    if ((pixel & (0x80 >> j)) != 0)
                    {
                        if (display[x + j + (y + i) * 64] == 1)
                        {
                            V[0xF] = 1;
                        }
                        display[x + j + (y + i) * 64] ^= 1;
                    }
                }
            }
            drawFlag = 1;
            break;
        case 0xE000:
            break;
        case 0xF000:
            break;
    }
}

void cleanupSDL()
{
    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
