# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -Wpedantic `pkg-config --cflags raylib gtk+-3.0 libavformat`
LIBS = `pkg-config --libs raylib gtk+-3.0 libavformat libavutil Magick++` -lglfw -lm -ldl -lpthread -lmagic

# Target executable
TARGET = raven
SRC = main.c

# Build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean up build files
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean
