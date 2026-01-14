CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lm
TARGET = main
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Source files
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/bmp.c $(SRC_DIR)/scaling.c $(SRC_DIR)/denoise.c $(SRC_DIR)/sharpen.c $(SRC_DIR)/edge.c $(SRC_DIR)/utils.c

# Object files
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/bmp.o $(OBJ_DIR)/scaling.o $(OBJ_DIR)/denoise.o $(OBJ_DIR)/sharpen.o $(OBJ_DIR)/edge.o $(OBJ_DIR)/utils.o

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/bmp.h $(INC_DIR)/scaling.h $(INC_DIR)/denoise.h $(INC_DIR)/sharpen.h $(INC_DIR)/edge.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/bmp.o: $(SRC_DIR)/bmp.c $(INC_DIR)/bmp.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/bmp.c -o $(OBJ_DIR)/bmp.o

$(OBJ_DIR)/scaling.o: $(SRC_DIR)/scaling.c $(INC_DIR)/scaling.h $(INC_DIR)/bmp.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/scaling.c -o $(OBJ_DIR)/scaling.o

$(OBJ_DIR)/denoise.o: $(SRC_DIR)/denoise.c $(INC_DIR)/denoise.h $(INC_DIR)/bmp.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/denoise.c -o $(OBJ_DIR)/denoise.o

$(OBJ_DIR)/sharpen.o: $(SRC_DIR)/sharpen.c $(INC_DIR)/sharpen.h $(INC_DIR)/denoise.h $(INC_DIR)/bmp.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/sharpen.c -o $(OBJ_DIR)/sharpen.o

$(OBJ_DIR)/edge.o: $(SRC_DIR)/edge.c $(INC_DIR)/edge.h $(INC_DIR)/bmp.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/edge.c -o $(OBJ_DIR)/edge.o

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(INC_DIR)/utils.h $(INC_DIR)/bmp.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/utils.c -o $(OBJ_DIR)/utils.o

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean