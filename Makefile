# Detect OS
ifeq ($(OS),Windows_NT)
    # Windows settings
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c99 -g
    LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
    TARGET_EXT = .exe
    RM = del /Q
    MKDIR = mkdir
    SEP = \\
else
    # Unix/Linux/macOS settings
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c99 -g
    LIBS = -lSDL2 -lSDL2_image
    TARGET_EXT =
    RM = rm -rf
    MKDIR = mkdir -p
    SEP = /
    
    # Check if we're on macOS and add Homebrew paths
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        # macOS with Homebrew paths
        HOMEBREW_PREFIX := $(shell brew --prefix)
        CFLAGS += -I$(HOMEBREW_PREFIX)/include
        LIBS := -L$(HOMEBREW_PREFIX)/lib $(LIBS)
    endif
endif

# Directories
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = main.c image_loader.c image_analysis.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)$(SEP)%.o)
TARGET = $(BINDIR)$(SEP)image_loader_demo$(TARGET_EXT)

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
ifeq ($(OS),Windows_NT)
	@if not exist "$(OBJDIR)" $(MKDIR) "$(OBJDIR)"
	@if not exist "$(BINDIR)" $(MKDIR) "$(BINDIR)"
else
	@$(MKDIR) $(OBJDIR) $(BINDIR)
endif

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
ifeq ($(OS),Windows_NT)
$(OBJDIR)$(SEP)%.o: $(SRCDIR)$(SEP)%.c
	$(CC) $(CFLAGS) -c $< -o $@
else
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
endif

# Clean build artifacts
clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJDIR)" rmdir /S /Q "$(OBJDIR)"
	@if exist "$(BINDIR)" rmdir /S /Q "$(BINDIR)"
else
	$(RM) $(OBJDIR) $(BINDIR)
endif
	@echo "Clean complete"

# Install SDL2 and SDL2_image (Windows with MSYS2/MinGW)
install-deps-windows:
	pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image

# Install using vcpkg (Windows alternative)
install-deps-vcpkg:
	vcpkg install sdl2 sdl2-image

# Install SDL2 and SDL2_image (Ubuntu/Debian)
install-deps-ubuntu:
	sudo apt-get update
	sudo apt-get install libsdl2-dev libsdl2-image-dev

# Install SDL2 and SDL2_image (macOS with Homebrew)
install-deps-macos:
	brew install sdl2 sdl2_image

# Install SDL2 and SDL2_image (Arch Linux)
install-deps-arch:
	sudo pacman -S sdl2 sdl2_image

# Run the program with a test image
test: $(TARGET)
	@echo "Running with test image (if available)..."
ifeq ($(OS),Windows_NT)
	@if exist "test.png" ( $(TARGET) test.png ) else if exist "test.jpg" ( $(TARGET) test.jpg ) else ( echo No test image found. Usage: $(TARGET) ^<image_file^> && $(TARGET) )
else
	@if [ -f "test.png" ]; then \
		./$(TARGET) test.png; \
	elif [ -f "test.jpg" ]; then \
		./$(TARGET) test.jpg; \
	else \
		echo "No test image found. Usage: ./$(TARGET) <image_file>"; \
		./$(TARGET); \
	fi
endif

# Debug build
debug: CFLAGS += -DDEBUG -g3
debug: clean all

# Release build
release: CFLAGS += -O3 -DNDEBUG
release: clean all

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build the project (default)"
	@echo "  clean            - Remove build artifacts"
	@echo "  test             - Build and run with test image"
	@echo "  debug            - Build debug version"
	@echo "  release          - Build optimized release version"
	@echo "  install-deps-windows - Install SDL2 dependencies using MSYS2/MinGW"
	@echo "  install-deps-vcpkg   - Install SDL2 dependencies using vcpkg"
	@echo "  install-deps-ubuntu  - Install SDL2 dependencies for Ubuntu/Debian"
	@echo "  help             - Show this help message"

.PHONY: all clean directories test debug release help install-deps-windows install-deps-vcpkg install-deps-ubuntu install-deps-macos install-deps-arch