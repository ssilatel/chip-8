CC = gcc
C_FLAGS = -Wall -Werror -pedantic
L_FLAGS = -lSDL2
OBJS = src/chip8.c
HEADERS = src/chip8.h
OBJ_NAME = chip8

all: $(OBJS)
	$(CC) $(OBJS) $(HEADERS) $(C_FLAGS) $(L_FLAGS) -o $(OBJ_NAME)
