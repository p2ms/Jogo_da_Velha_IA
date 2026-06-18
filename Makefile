CC      = cc
CFLAGS  = -O2 -Wall -Wextra -std=c11

COMMON_OBJS = board.o minimax.o alphabeta.o astar.o nodes.o

all: benchmark

benchmark: benchmark.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ benchmark.o $(COMMON_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o benchmark

.PHONY: all clean
