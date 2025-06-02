CC=g++ # define the compiler to use
TARGET=slab # define the name of the executable
CFLAGS=-O3
LFLAGS+= -Wl,--defsym,__stack_limit=3000000000
override SOURCES := $(shell find . -type f -name '*.c')
# define list of objects
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS)  -o $(TARGET) $(SOURCES) 

$(OBJS): %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o slab
