# Makefile for wazan

CC = gcc
CFLAGS = -Wall -O2 -Iinclude
TARGET = wazan
SRC = src/wazan.c
PREFIX = /usr/local

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin/
	@echo "wazan installed to $(PREFIX)/bin/"
