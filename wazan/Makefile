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
	rm -f src/*.o
	rm -f include/*.gch
	sudo rm -f /usr/local/bin/$(TARGET) 2>/dev/null || true
	@echo "Cleaned build files and old binary."

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin/
	@echo "wazan installed to $(PREFIX)/bin/"
