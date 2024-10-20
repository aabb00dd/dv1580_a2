#include "memory_manager.h"
#include <pthread.h>

static char *memory_pool = NULL;      // Pointer to the memory pool for data
static char *header_pool = NULL;      // Pointer to the memory pool for headers (Block structures)
static Block *free_list = NULL;       // Pointer to the list of free blocks
static pthread_mutex_t memory_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex for synchronizing memory operations


/*
 Initializes the memory manager by allocating pools of memory.
 param: size The total size of the memory pool to be created for data.
 This function allocates memory for both the data pool and the header pool, and sets up the free list.
 note: Exits the program if memory allocation fails.
 */
void mem_init(size_t size) 
{
    memory_pool = (char*)malloc(size);        // Allocate memory for the data pool
    header_pool = (char*)malloc(size);  // Allocate memory for the header pool

    // Exit if memory allocation fails
    if (!memory_pool || !header_pool) 
    {
        exit(EXIT_FAILURE);
    }

    // Initialize the free block
    free_list = (Block*)header_pool;          // Set the free list to the start of the header pool
    free_list->size_of_block = size;          // Entire pool size
    free_list->is_free = 1;                   // Mark as free
    free_list->next_block = NULL;             // No next block
    free_list->data = memory_pool;            // Pointer to the data in the block
}

/*
 Allocates a block of memory of at least the specified size.
 The allocation uses first-fit strategy to find a suitable free block.
 param: size The size of the memory block to allocate in bytes.
 return: Pointer to the allocated memory, or NULL if allocation fails.
 */
void* mem_alloc(size_t size) 
{
    pthread_mutex_lock(&memory_lock); // Lock the memory manager

    if (size == 0)
    {
        // For zero-size allocation, return the first free block's data pointer
        Block *current_block = free_list;
        while (current_block) 
        {
            if (current_block->is_free) 
            {
                pthread_mutex_unlock(&memory_lock);
                return current_block->data;
            }
            current_block = current_block->next_block;
        }
        // If no free block found, return NULL
        pthread_mutex_unlock(&memory_lock);
        return NULL;
    }

    // Pointers to traverse and track the free list
    Block *current_block = free_list, *prev_block = NULL;

    // Find a free block that fits the requested size
    while (current_block) 
    {
        // Check if the current block is free and can accommodate the requested size.
        if (current_block->is_free && current_block->size_of_block >= size) 
        {
            // If the current block is larger than needed, split it into two blocks.
            if (current_block->size_of_block > size + sizeof(Block)) 
            {
                // Allocate a new block from the header pool for the split block
                Block *new_block = (Block*)((char*)current_block + sizeof(Block));
                new_block->size_of_block = current_block->size_of_block - size;           // Set size of the new block
                new_block->is_free = 1;                                                  // Mark as free
                new_block->next_block = current_block->next_block;                       // Link the new block to the next one in the list
                new_block->data = current_block->data + size;                              // Set the data pointer for the new block
                
                current_block->size_of_block = size;                                     // Adjust the current block to the requested size
                current_block->next_block = new_block;                                   // Link the current block to the newly created one
            }

            current_block->is_free = 0; // Mark as not free

            // Link the previous block to the next block if there is one.
            if (prev_block) prev_block->next_block = current_block->next_block;
            // If no previous block, this becomes the new head of the free list.
            else free_list = current_block->next_block;

            pthread_mutex_unlock(&memory_lock); // Unlock the memory manager
            // Return a pointer to the allocated memory block
            return current_block->data;
        }

        // Move to the next block
        prev_block = current_block;
        current_block = current_block->next_block;
    }

    pthread_mutex_unlock(&memory_lock); // Unlock if no suitable block is found
    return NULL; // No suitable block found
}

/*
 Frees a previously allocated memory block.
 param: block Pointer to the memory block to be freed.
 note: If the pointer is NULL or the block is already freed, this function does nothing.
 */
void mem_free(void* block) 
{
    pthread_mutex_lock(&memory_lock); // Unlock if already free
    
    if (!block){
        pthread_mutex_unlock(&memory_lock); // Unlock if block is NULL
        return;    // Attempted to free a NULL pointer
    }

    // Find the block in the header pool
    Block *block_to_free = free_list;
    Block *prev_block = NULL;

    while (block_to_free) 
    {
        if (block_to_free->data == block) 
        {
            if (block_to_free->is_free) 
            {
                pthread_mutex_unlock(&memory_lock);
                return; // Block is already free
            }

            block_to_free->is_free = 1;

            // Merge with next block if it's free
            if (block_to_free->next_block && block_to_free->next_block->is_free) 
            {
                block_to_free->size_of_block += sizeof(Block) + block_to_free->next_block->size_of_block;
                block_to_free->next_block = block_to_free->next_block->next_block;
            }

            // Merge with previous block if it's free
            if (prev_block && prev_block->is_free) 
            {
                prev_block->size_of_block += sizeof(Block) + block_to_free->size_of_block;
                prev_block->next_block = block_to_free->next_block;
            } 
            else 
            {
                // If not merged with previous, ensure this block is in the free list
                block_to_free->next_block = free_list;
                free_list = block_to_free;
            }

            pthread_mutex_unlock(&memory_lock);
            return;
        }

        prev_block = block_to_free;
        block_to_free = block_to_free->next_block;
    }

    // If we get here, the block was not found
    pthread_mutex_unlock(&memory_lock);
    // Consider logging an error here
}


/*
 Resizes a memory block to the specified size.
 param: block Pointer to the memory block to be resized.
 param: size The new size for the memory block in bytes.
 return: A pointer to the resized memory block, or NULL if the resizing operation fails.
 */
void* mem_resize(void* block, size_t size) 
{
    pthread_mutex_lock(&memory_lock);

    if (!block) {
        void* result = mem_alloc(size);
        pthread_mutex_unlock(&memory_lock);
        return result;
    }

    if (size == 0) 
    {
        mem_free(block);
        pthread_mutex_unlock(&memory_lock);
        return NULL;
    }

    // Get the old block and check if it's big enough
    Block* old_block = (Block*)((char*)block - sizeof(Block));

    if (old_block->size_of_block >= size) {
        pthread_mutex_unlock(&memory_lock);
        return block;
    }

    // Allocate a new block of the requested size
    void* new_block = mem_alloc(size);

    // Copy the old data to the new block and free the old block
    if (new_block) 
    {
        memcpy(new_block, block, old_block->size_of_block);  // Copy old data to new block
        mem_free(block);                                     // Free the old block
    }
    
    pthread_mutex_unlock(&memory_lock);
    return new_block; // Return the new block
}


/*
 Deinitializes the memory manager and frees the memory pools.
 note: This function should be called when memory management is no longer needed.
 */
void mem_deinit() 
{
    pthread_mutex_lock(&memory_lock); // Lock the memory manager
    free(memory_pool);  // Free the data memory pool
    free(header_pool);  // Free the header memory pool
    memory_pool = NULL; // Reset the memory pool pointer
    header_pool = NULL; // Reset the header pool pointer
    free_list = NULL;   // Reset the free list pointer

    pthread_mutex_unlock(&memory_lock); // Unlock before destroying the mutex
    pthread_mutex_destroy(&memory_lock); // Destroy the mutex
}
