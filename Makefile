CC      = gcc
CFLAGS  = -Wall -O2 -Iinclude
LDFLAGS = -pthread

SRCS    = main.c src/cJson.c src/leitor.c src/processador.c src/visualizacao.c src/fila.c src/logger.c
OBJS    = $(SRCS:.c=.o)
TARGET  = analise-de-sensores

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
