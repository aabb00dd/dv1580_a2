# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC
LIB_NAME = libmemory_manager.so

# Source and Object Files
SRC = memory_manager.c
OBJ = $(SRC:.c=.o)

# Default target: Build both memory manager and linked list
all: mmanager list

# Rule to create the dynamic library for memory manager
$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $@ $(OBJ)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager as a dynamic library
mmanager: $(LIB_NAME)

# Build the linked list and link with the memory manager library
list: linked_list.o
	$(CC) -o linked_list linked_list.o -L. -lmemory_manager -lpthread -lm

# Test target to run the memory manager test program
test_mmanager: $(LIB_NAME)
	$(CC) -o test_memory_manager test_memory_manager.c -L. -lmemory_manager -lpthread -lm

# Test target to run the linked list test program
test_list: $(LIB_NAME) linked_list.o
	$(CC) -o test_linked_list linked_list.o test_linked_list.c -L. -lmemory_manager -lpthread -lm
	
# Run both test cases
run_tests: run_test_mmanager run_test_list

# Run test cases for the memory manager
run_test_mmanager:
	./test_memory_manager

# Run test cases for the linked list
run_test_list:
	./test_linked_list

# Clean target to remove all build files
clean:
	rm -f $(OBJ) $(LIB_NAME) test_memory_manager test_linked_list linked_list.o linked_list
