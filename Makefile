# Object files
OBJS = main.o shared_memory.o monitor.o receptionist.o visitor.o

# Source files
SOURCE = main.cpp shared_memory.cpp monitor.cpp receptionist.cpp visitor.cpp

# Header files
HEADER = shared_memory.h

# Output program names
OUT_MAIN = init
OUT_MONITOR = monitor
OUT_RECEPTIONIST = receptionist
OUT_VISITOR = visitor

# Compiler and flags
CC = g++
FLAGS = -g -c
LIBS = -lpthread -lrt  # Add pthread and real-time libraries

# Default target: build all executables
all: $(OUT_MAIN) $(OUT_MONITOR) $(OUT_RECEPTIONIST) $(OUT_VISITOR)

# Main program for initializing shared memory and semaphores
$(OUT_MAIN): main.o shared_memory.o
	$(CC) -g main.o shared_memory.o -o $(OUT_MAIN) $(LIBS)

# Monitor program
$(OUT_MONITOR): monitor.o shared_memory.o
	$(CC) -g monitor.o shared_memory.o -o $(OUT_MONITOR) $(LIBS)

# Receptionist program
$(OUT_RECEPTIONIST): receptionist.o shared_memory.o
	$(CC) -g receptionist.o shared_memory.o -o $(OUT_RECEPTIONIST) $(LIBS)

# Visitor program
$(OUT_VISITOR): visitor.o shared_memory.o
	$(CC) -g visitor.o shared_memory.o -o $(OUT_VISITOR) $(LIBS)

# Individual object file compilation rules
main.o: main.cpp
	$(CC) $(FLAGS) main.cpp

shared_memory.o: shared_memory.cpp shared_memory.h
	$(CC) $(FLAGS) shared_memory.cpp

monitor.o: monitor.cpp shared_memory.h
	$(CC) $(FLAGS) monitor.cpp

receptionist.o: receptionist.cpp shared_memory.h
	$(CC) $(FLAGS) receptionist.cpp

visitor.o: visitor.cpp shared_memory.h
	$(CC) $(FLAGS) visitor.cpp

# Clean up compiled files
clean:
	rm -f $(OBJS) $(OUT_MAIN) $(OUT_MONITOR) $(OUT_RECEPTIONIST) $(OUT_VISITOR)

# Count lines of code
count:
	wc $(SOURCE) $(HEADER)
