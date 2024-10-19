#include "memory_manager.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static char *memory_pool = NULL;    // Pointer to the memory pool
static Block *free_list = NULL;     // Pointer to the list of free blocks
static pthread_mutex_t memory_lock; // Mutex for synchronizing memory operations

/* Initializes the memory manager by allocating a pool of memory. */
void mem_init(size_t size) 
{
    memory_pool = (char*)malloc(size); // Allocate memory for the pool

    if (!memory_pool) 
    {
        exit(EXIT_FAILURE);  // Exit if memory allocation fails
    }

    // Initialize the free block at the start of the memory pool
    free_list = (Block*)memory_pool;
    free_list->size_of_block = size;
    free_list->is_free = 1;  // Mark block as free
    free_list->next_block = NULL;  // No next block

    // Initialize the mutex
    pthread_mutex_init(&memory_lock, NULL);
}

/* Allocates a block of memory of at least the specified size. */
void* mem_alloc(size_t size) 
{
    if (size == 0) return NULL;  // Requested size is zero

    pthread_mutex_lock(&memory_lock); // Lock memory manager

    Block *current_block = free_list, *prev_block = NULL;

    // Traverse free list to find a suitable block
    while (current_block) 
    {
        if (current_block->is_free && current_block->size_of_block >= size + sizeof(Block)) 
        {
            // Block splitting logic
            if (current_block->size_of_block > size) 
            {
                Block *new_block = (Block*)((char*)current_block + sizeof(Block) + size);
                new_block->size_of_block = current_block->size_of_block - size;
                new_block->is_free = 1;
                new_block->next_block = current_block->next_block;

                current_block->size_of_block = size;
                current_block->next_block = new_block;
            }

            current_block->is_free = 0;  // Mark block as allocated
            pthread_mutex_unlock(&memory_lock); // Unlock memory manager
            return (char*)current_block + sizeof(Block);
        }
        prev_block = current_block;
        current_block = current_block->next_block;
    }

    pthread_mutex_unlock(&memory_lock);  // Unlock in case of failure
    return NULL;  // No suitable block found
}

/* Frees the previously allocated block of memory. */
void mem_free(void *block) 
{
    if (!block) return;

    pthread_mutex_lock(&memory_lock); // Lock memory manager

    Block *block_to_free = (Block*)((char*)block - sizeof(Block));
    block_to_free->is_free = 1;  // Mark block as free

    // Coalesce adjacent free blocks
    Block *current_block = free_list;
    while (current_block && current_block->next_block) 
    {
        if (current_block->is_free && current_block->next_block->is_free) 
        {
            current_block->size_of_block += sizeof(Block) + current_block->next_block->size_of_block;
            current_block->next_block = current_block->next_block->next_block;
        }
        current_block = current_block->next_block;
    }

    pthread_mutex_unlock(&memory_lock);  // Unlock memory manager
}

/* Resizes a memory block to the specified size. */
void* mem_resize(void* block, size_t size) 
{
    if (!block) return mem_alloc(size);  // If block is NULL, allocate new

    if (size == 0) 
    {
        mem_free(block);
        return NULL;
    }

    pthread_mutex_lock(&memory_lock);  // Lock memory manager

    Block* old_block = (Block*)((char*)block - sizeof(Block));
    if (old_block->size_of_block >= size) 
    {
        pthread_mutex_unlock(&memory_lock);  // Unlock if no resizing is needed
        return block;
    }

    void* new_block = mem_alloc(size);
    if (new_block) 
    {
        memcpy(new_block, block, old_block->size_of_block);
        mem_free(block);  // Free the old block
    }

    pthread_mutex_unlock(&memory_lock);  // Unlock memory manager
    return new_block;
}

/* Deinitializes the memory manager and frees the memory pool. */
void mem_deinit() 
{
    pthread_mutex_lock(&memory_lock); // Lock memory manager

    free(memory_pool);  // Free the memory pool
    memory_pool = NULL;
    free_list = NULL;

    pthread_mutex_unlock(&memory_lock);  // Unlock before destroying mutex
    pthread_mutex_destroy(&memory_lock);  // Destroy the mutex
}
