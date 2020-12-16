CC = gcc
CFLAGS = -Wall -std=c99 
LDLIBS = -lmingw32 -lSDL2main -lSDL2 -lm 

mandel.exe:mandel.c 
	$(CC) $(CFLAGS)  $^ $(LDLIBS) -o $@
