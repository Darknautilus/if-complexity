SRC=voyCom_a_distribuer.c
OBJ=$(SRC:.c=.o)

EXEC=voyage
CC=gcc

CFLAGS=
LDFLAGS= -lrt -lm

all: $(EXEC)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	@rm -rf *.o

mrproper: clean
	@rm -rf $(EXEC)
