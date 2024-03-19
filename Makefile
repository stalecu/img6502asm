CC = gcc
CFLAGS = -Wall -Wextra -Werror -I. -std=c99 -lc -lm -lpng

SRCDIR = .
SOURCES = img6502asm.c main.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = img6502asm

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXECUTABLE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)
