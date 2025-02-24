# Detect OS and set correct shared library file extension
UNAME_S := $(shell uname -s)

# Compiler and flags
CC = gcc
ifeq ($(UNAME_S),Linux)
    CFLAGS = -g -Wall -Wextra -fPIC -I ./ -I ../HiGHS/build/ -I ../HiGHS/src/ -I ../HiGHS/src/interfaces/
else ifeq ($(UNAME_S),Darwin)
    CFLAGS = -g -Wall -Wextra -fPIC -I ./ -I ../HiGHS/build/ -I ../HiGHS/src/ -I ../HiGHS/src/interfaces/
else
    $(error Unsupported OS: $(UNAME_S))
endif

ifeq ($(UNAME_S),Linux)
    LDFLAGS = -L../HiGHS/build/lib
else ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -L../HiGHS/build/lib
else
    $(error Unsupported OS: $(UNAME_S))
endif


# Source files
SRCS = json_processor.c linprog.c

# Object files
OBJS = $(SRCS:.c=.o)

# Static linking of HiGHS
#ifeq ($(UNAME_S),Linux)
#    LDFLAGS = 
#    HIGHS_LIB =   # Statically link HiGHS
#else ifeq ($(UNAME_S),Darwin)
#    LDFLAGS = -L../HiGHS/build/lib -L/opt/homebrew/lib
#    HIGHS_LIB = ../HiGHS/build/lib/libhighs.a  # Statically link HiGHS
#else
#    $(error Unsupported OS: $(UNAME_S))
#endif

LDLIBS = -lhighs -lm -ljansson  # Link the static HiGHS library

ifeq ($(UNAME_S),Linux)
    SHARED_LIB = libjson_processor.so
    SHARED_FLAGS = -shared
else ifeq ($(UNAME_S),Darwin)
    SHARED_LIB = libjson_processor.dylib
    SHARED_FLAGS = -dynamiclib
else
    $(error Unsupported OS: $(UNAME_S))
endif

# Default rule to build everything
all: $(SHARED_LIB)

# Rule to compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create shared library (automatically detects OS)
$(SHARED_LIB): $(OBJS)
	$(CC) $(SHARED_FLAGS) -o $(SHARED_LIB) $(OBJS) $(LDFLAGS) $(LDLIBS)

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET) $(SHARED_LIB)

# Rebuild everything
rebuild: clean all
