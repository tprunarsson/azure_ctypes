CC = gcc
CFLAGS = -Wall -fPIC -I/opt/homebrew/include
LDFLAGS = -shared -L/opt/homebrew/lib -ljansson
TARGET = libjson_processor.so

# Build the shared library
$(TARGET): json_processor.c
	$(CC) $(CFLAGS) -o $(TARGET) json_processor.c $(LDFLAGS)

# Clean up generated files
clean:
	rm -f $(TARGET)

