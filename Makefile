CC=gcc # define the compiler to use
TARGET=slab # define the name of the executable
CFLAGS+=-g -Wno-builtin-declaration-mismatch
override SOURCES := $(shell find . -type f -name '*.c')

# the target is obtained linking all .o files
all:  $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS)  -o $(TARGET) $(SOURCES) 

clean:
	rm -f *.o slab
