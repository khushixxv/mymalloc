CC = gcc
CFLAGS = -g -Wall -Werror -Wvla -fsanitize=address -std=c99
DEPS = mymalloc.h
OBJ = mymalloc.o memperf.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

memperf: $(OBJ)
	gcc $(CFLAGS) -o $@ $^