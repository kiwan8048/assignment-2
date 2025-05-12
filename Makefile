# Makefile

CC = gcc
CFLAGS = -Wall -g
TARGET = myshell
SRCS = shell.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
