#include "memory_manager.h"


static char *memory_pool = NULL;    // Pointer to the memory pool
static Block *free_list = NULL;     // Pointer to the list of free blocks


/*
 Initializes the memory manager by allocating a pool of memory.
 param: size The total size of the memory pool to be created.
 This function allocates memory for the pool and sets up the free list.
 note: Exits the program if memory allocation fails.
 */
void mem_init(size_t size) 
{
    memory_pool = (char*)malloc(size); // Allocate memory for the pool

    // Exit if memory allocation fails
    if (!memory_pool) 
    {
        exit(EXIT_FAILURE);
    }

    // Initialize the free block
    free_list = (Block*)memory_pool; // Set the free list to the start of the pool
    free_list->size_of_block = size; // Entire pool size
    free_list->is_free = 1;          // Mark as free
    free_list->next_block = NULL;    // No next block
}


/*
 Allocates a block of memory of at least the specified size.
 The allocation uses first-fit strategy to find a suitable free block.
 param: size The size of the memory block to allocate in bytes.
 return: Pointer to the allocated memory, or NULL if allocation fails.
 note: Returns NULL if the requested size is zero or if no suitable
 free block is found.
 */
void* mem_alloc(size_t size) 
{
    if (size == 0) return NULL;    // Requested size is zero

    // Pointers to traverse and track the free list
    Block *current_block = free_list, *prev_block = NULL;

    // Find a free block that fits the requested size
    while (current_block) 
    {
        // Check if the current_block block is free and can accommodate the requested size.
        if (current_block->is_free && current_block->size_of_block >= size) 
        {
            // If the current_block block is larger than needed, split it into two blocks.
            if (current_block->size_of_block > size) 
            {
                Block *new_block = (Block*)((char*)current_block + sizeof(Block) + size); // Create a new block after the allocated block.
                new_block->size_of_block = current_block->size_of_block - size;           // Set size of the new block.
                new_block->is_free = 1;                                                   // Mark as free
                new_block->next_block = current_block->next_block;                        // Link the new block to the next one in the list.
                current_block->size_of_block = size;                                      // Adjust the current block to the requested size.
                current_block->next_block = new_block;                                    // Link the current block to the newly created one.
            }

            current_block->is_free = 0; // Mark as not free

            // Link the previous block to the next block if there is one.
            if (prev_block) prev_block->next_block = current_block->next_block;
            // If no previous block, this becomes the new head of the free list.
            else free_list = current_block->next_block;

            // Return a pointer to the allocated memory block
            return (char*)current_block + sizeof(Block);
        }

        // Move to the next block
        prev_block = current_block;
        current_block = current_block->next_block;
    }

    return NULL; // No suitable block found
}


/*
 Frees a previously allocated memory block.
 param: block Pointer to the memory block to be freed.
 note: If the pointer is NULL or the block is already freed, this function does nothing.
 */
void mem_free(void* block) 
{
    if (!block) return;    // Attempted to free a NULL pointer

    // Get the block to free
    Block* block_to_free = (Block*)((char*)block - sizeof(Block));

    // Attempted to free an already freed block
    if (block_to_free->is_free) return;

    // Mark the block as free
    block_to_free->is_free = 1;
    // Pointers to traverse and track the free list
    Block *current_block = free_list, *prev_block = NULL;

    // Find the correct position to insert the freed block in the free list
    while (current_block && current_block < block_to_free) 
    {
        prev_block = current_block;                     // Track the previous block
        current_block = current_block->next_block;      // Move to the next block
    }

    // Insert the freed block into the free list
    block_to_free->next_block = current_block;

    // Link the previous block to the freed block
    if (prev_block) prev_block->next_block = block_to_free;
    // If no previous block, this becomes the new head of the free list.
    else free_list = block_to_free;

    // Merge with the next block if they are contiguous
    if (block_to_free->next_block && (char*)block_to_free + sizeof(Block) + block_to_free->size_of_block == (char*)block_to_free->next_block) 
    {
        block_to_free->size_of_block += sizeof(Block) + block_to_free->next_block->size_of_block;              // Merge the two blocks
        block_to_free->next_block = block_to_free->next_block->next_block;                                     // Update the next pointer
    }

    // Merge with the previous block if they are contiguous
    if (prev_block && (char*)prev_block + sizeof(Block) + prev_block->size_of_block == (char*)block_to_free) 
    {
        prev_block->size_of_block += sizeof(Block) + block_to_free->size_of_block;              // Merge the two blocks
        prev_block->next_block = block_to_free->next_block;                                     // Update the next pointer
    }
}


/*
 Resizes a memory block to the specified size.
 param: block Pointer to the memory block to be resized.
 param: size The new size for the memory block in bytes.
 return: A pointer to the resized memory block, or NULL if the resizing operation fails.
 */
void* mem_resize(void* block, size_t size) 
{
    if (!block) return mem_alloc(size);    // If block is NULL, allocate new

    // If size is zero, free the block
    if (size == 0) 
    {
        mem_free(block);
        return NULL;
    }

    // Get the old block and check if it's big enough
    Block* old_block = (Block*)((char*)block - sizeof(Block));
    if (old_block->size_of_block >= size) return block;

    // Allocate a new block of the requested size
    void* new_block = mem_alloc(size);

    // Copy the old data to the new block and free the old block
    if (new_block) 
    {
        memcpy(new_block, block, old_block->size_of_block);  // Copy old data to new block
        mem_free(block);                                     // Free the old block
    }
    
    return new_block; // Return the new block
}


/*
 Deinitializes the memory manager and frees the memory pool.
 note: This function should be called when memory management is no longer needed.
 */
void mem_deinit() 
{
    free(memory_pool);  // Free the memory pool
    memory_pool = NULL; // Reset the memory pool pointer
    free_list = NULL;   // Reset the free list pointer
}
