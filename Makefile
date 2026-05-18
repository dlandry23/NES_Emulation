CC = gcc
CFLAGS = -g -Wall
SRC = $(wildcard *.c)
TARGET = NES_EMULATION.exe

all: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)