CC=gcc # define the compiler to use
TARGET=slab # define the name of the executable
CFLAGS+=-g -Wno-builtin-declaration-mismatch
override SOURCES := $(shell find . -type f -name '*.c')

all:  $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS)  -o $(TARGET) $(SOURCES) 

clean:
	rm -f slab
