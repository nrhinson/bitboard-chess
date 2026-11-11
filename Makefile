# Makefile for Bitboard Chess
#
#   make        build the game
#   make run    build (if needed) and play
#   make clean  delete the built program
#
# Requires GCC 13 or newer (the code uses a C23 enum with a uint64_t base type).

CC      = gcc
CFLAGS  = -Wall -Wextra -O2
SRC     = Chessboard.c
TARGET  = chess

# Windows (cmd/PowerShell with mingw32-make): builds chess.exe and uses del.
# If you run make from a Unix-style shell on Windows (MSYS2 or Git Bash) and
# 'make clean' or 'make run' fails, use:  make run RM="rm -f" RUN=./chess.exe
ifeq ($(OS),Windows_NT)
    EXE = $(TARGET).exe
    RM  = del /Q
    RUN = .\$(EXE)
else
    EXE = $(TARGET)
    RM  = rm -f
    RUN = ./$(EXE)
endif

.PHONY: all run clean

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE)

run: $(EXE)
	$(RUN)

clean:
	-$(RM) $(EXE)
