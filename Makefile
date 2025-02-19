CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=gnu99 -lm
LDFLAGS = -lm

SRCS = main.c datastructs.c memory.c
EXE = allocate

all: $(EXE)

$(EXE): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean: 
	rm -f $(EXE)
