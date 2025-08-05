MAKEFLAGS += --no-print-directory -s

# Compiler and flags
CC := clang
BASE_CFLAGS := -std=c23 -Wpedantic -fno-common \
              -fno-plt -fno-semantic-interposition -fstrict-enums \
              -fstrict-return -fvisibility=hidden -fstack-protector-strong \
              -ftrivial-auto-var-init=pattern -fstrict-flex-arrays=3 -Wall -Wextra -Wdouble-promotion -Wformat=2 -Wnull-dereference \
                -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
                -Wshadow -Wundef -Wfloat-equal -Wcast-align -Wpointer-arith \
                -Wwrite-strings -Wunused-parameter -Wpacked \
                -Wpadded -Wredundant-decls -Wcast-qual \
                -Wconversion -Wswitch-default -Wswitch-enum -Wno-bool-operation
LDFLAGS :=
CPPFLAGS :=

# Directories
SRCDIR := src
INCDIR := include
BINDIR := bin

# Files
SOURCES := $(wildcard $(SRCDIR)/*.c)

# Default build type
BUILD_TYPE ?= dev

# Conditionally define directories and flags based on BUILD_TYPE
ifeq ($(BUILD_TYPE),debug)
	TARGET := $(BINDIR)/main-debug.exe
	CFLAGS := $(BASE_CFLAGS) -DDEBUG -O0
else ifeq ($(BUILD_TYPE),release)
	TARGET := $(BINDIR)/main-release.exe
	CFLAGS := $(BASE_CFLAGS) -DNDEBUG -O3 -flto -pipe
else
	TARGET := $(BINDIR)/main-dev.exe
	CFLAGS := $(BASE_CFLAGS) -O2
endif

# Default target
.PHONY: all clean build debug release install help distclean tags print-vars

all: build

# Create bin directory if it doesn't exist
$(BINDIR):
	@mkdir -p $@

# Link executable directly from sources
$(TARGET): $(SOURCES) | $(BINDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(SOURCES) -o $@

# Generate compile_commands.json for clangd
compile_commands.json: $(SOURCES)
	@echo "[" > $@.tmp
	@count=0; \
	for src in $(SOURCES); do \
		if [ $$count -gt 0 ]; then \
			echo "," >> $@.tmp; \
		fi; \
		echo "  {" >> $@.tmp; \
		echo "	\"directory\": \"$$(pwd)\"," >> $@.tmp; \
		echo "	\"command\": \"$(CC) $(CFLAGS) $(CPPFLAGS) -c $$src -o /dev/null\"," >> $@.tmp; \
		echo "	\"file\": \"$$src\"" >> $@.tmp; \
		echo "  }" >> $@.tmp; \
		count=$$((count + 1)); \
	done
	@echo "]" >> $@.tmp
	@mv $@.tmp $@

# Build with compile_commands.json
build: compile_commands.json $(TARGET)

# Clean build files
clean:
	@rm -rf $(BINDIR) compile_commands.json

# Very clean - remove all generated files
distclean: clean
	@rm -f tags cscope.*

# Generate tags for navigation (optional)
tags: $(SOURCES)
	@ctags -R $(SRCDIR) $(INCDIR)

run: $(TARGET)
	@./$(TARGET)

# Help target
help:
	@echo "Available targets:"
	@echo "  all	   - Build default (dev)"
	@echo "  build	 - Build with compile_commands.json"
	@echo "  debug	 - Build with debug flags"
	@echo "  release   - Build with optimization flags"
	@echo "  clean	 - Remove build files"
	@echo "  distclean - Remove all generated files"
	@echo "  tags	  - Generate ctags file"
	@echo "  help	  - Show this help"

# Print variables for debugging
print-vars:
	@echo "BUILD_TYPE = $(BUILD_TYPE)"
	@echo "SOURCES = $(SOURCES)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CPPFLAGS = $(CPPFLAGS)"

# Specific build targets
debug:
	@$(MAKE) BUILD_TYPE=debug

release:
	@$(MAKE) BUILD_TYPE=release

.DEFAULT_GOAL := build