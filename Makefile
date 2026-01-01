CC = gcc
CFLAGS = -Wall -Wextra -O2 -lm
TARGET = main
OBJS = main.o bmp.o filters.o utils.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS)

main.o: main.c bmp.h filters.h
	$(CC) -c main.c $(CFLAGS)

bmp.o: bmp.c bmp.h
	$(CC) -c bmp.c $(CFLAGS)

filters.o: filters.c filters.h bmp.h utils.h
	$(CC) -c filters.c $(CFLAGS)

utils.o: utils.c utils.h
	$(CC) -c utils.c $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean