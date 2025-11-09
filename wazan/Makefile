# Makefile for Wazan Language Interpreter

CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
TARGET = wazan_interp
SRCDIR = src
INCDIR = include
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all clean run test install install-user uninstall uninstall-user build-only

all: $(TARGET) install

build-only: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "ビルド完了: $(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "クリーンアップ完了"

run: $(TARGET)
	./$(TARGET) ../sample.wz

test: $(TARGET)
	@echo "=== テスト実行 ==="
	./$(TARGET) ../test.wz

install: $(TARGET)
	@if [ -w /usr/local/bin ]; then \
		cp $(TARGET) /usr/local/bin/wazan; \
		echo "インストール完了: /usr/local/bin/wazan"; \
	else \
		sudo cp $(TARGET) /usr/local/bin/wazan; \
		echo "インストール完了: /usr/local/bin/wazan (sudo使用)"; \
	fi
	@echo "使用方法: wazan <ファイル名.wz>"

install-user: $(TARGET)
	mkdir -p $(HOME)/.local/bin
	cp $(TARGET) $(HOME)/.local/bin/wazan
	@echo "インストール完了: $(HOME)/.local/bin/wazan"
	@echo "PATHに $(HOME)/.local/bin を追加してください"
	@echo "  export PATH=\"\$$HOME/.local/bin:\$$PATH\""

uninstall:
	sudo rm -f /usr/local/bin/wazan
	@echo "アンインストール完了"

uninstall-user:
	rm -f $(HOME)/.local/bin/wazan
	@echo "ユーザーインストールをアンインストールしました"
