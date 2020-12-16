CC = gcc
CFLAGS = -Wall -std=c99 
LDLIBS = -lmingw32 -lSDL2main -lSDL2 -lm 

mandle.exe:mandle.c 
	$(CC) $(CFLAGS)  $^ $(LDLIBS) -o $@
