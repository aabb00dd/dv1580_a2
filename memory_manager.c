#include "memory_manager.h"

static char *memory_pool = NULL;    // Pointer to the memory pool
static Block *free_list = NULL;     // Pointer to the list of free blocks
static Block *metadata_pool = NULL; // Separate pool for metadata

/*
 Initializes the memory manager by allocating a pool of memory.
 */
void mem_init(size_t size) 
{
    memory_pool = (char*)malloc(size); // Allocate memory for the pool
    metadata_pool = (Block*)malloc(sizeof(Block) * (size / sizeof(Block))); // Allocate memory for metadata blocks

    // Exit if memory allocation fails
    if (!memory_pool || !metadata_pool) 
    {
        exit(EXIT_FAILURE);
    }

    // Initialize the first free block using metadata
    free_list = metadata_pool;       // Assign the first block in metadata pool as the free list
    free_list->size_of_block = size; // The whole pool is initially free
    free_list->is_free = 1;          // Mark as free
    free_list->next_block = NULL;    // No next block
}

/*
 Allocates a block of memory of at least the specified size.
 */
void* mem_alloc(size_t size) 
{
    if (size == 0) return NULL;    // Return NULL if requested size is zero

    // Traverse the free list to find a suitable block
    Block *current_block = free_list, *prev_block = NULL;

    while (current_block) 
    {
        if (current_block->is_free && current_block->size_of_block >= size) 
        {
            // If the block is larger than needed, split it
            if (current_block->size_of_block > size) 
            {
                Block *new_block = (Block*)((char*)memory_pool + sizeof(Block) + size); // Create a new block after allocated space
                new_block->size_of_block = current_block->size_of_block - size;          // Adjust the size of the new block
                new_block->is_free = 1;                                                 // Mark the new block as free
                new_block->next_block = current_block->next_block;                      // Link the new block to the list
                current_block->size_of_block = size;                                    // Update current block size
                current_block->next_block = new_block;                                  // Link to the new block
            }

            current_block->is_free = 0; // Mark the current block as not free

            // Return a pointer to the allocated memory block (after metadata)
            return (char*)current_block + sizeof(Block);
        }

        prev_block = current_block;       // Move to the next block
        current_block = current_block->next_block;
    }

    return NULL; // No suitable block found
}

/*
 Frees a previously allocated memory block.
 */
void mem_free(void* block) 
{
    if (!block) return; // If NULL, do nothing

    // Get the block to free by adjusting for metadata size
    Block* block_to_free = (Block*)((char*)block - sizeof(Block));

    // If already free, do nothing
    if (block_to_free->is_free) return;

    // Mark the block as free
    block_to_free->is_free = 1;

    // Traverse the free list to insert the freed block in the right place
    Block *current_block = free_list, *prev_block = NULL;

    while (current_block && current_block < block_to_free) 
    {
        prev_block = current_block;      // Track the previous block
        current_block = current_block->next_block; // Move to the next block
    }

    // Insert the block into the free list
    block_to_free->next_block = current_block;

    // Update the previous block's pointer
    if (prev_block) 
        prev_block->next_block = block_to_free;
    else 
        free_list = block_to_free; // If there's no previous block, it becomes the new head of the free list

    // Merge with the next block if contiguous
    if (block_to_free->next_block && 
        (char*)block_to_free + sizeof(Block) + block_to_free->size_of_block == (char*)block_to_free->next_block) 
    {
        block_to_free->size_of_block += sizeof(Block) + block_to_free->next_block->size_of_block;
        block_to_free->next_block = block_to_free->next_block->next_block;
    }

    // Merge with the previous block if contiguous
    if (prev_block && 
        (char*)prev_block + sizeof(Block) + prev_block->size_of_block == (char*)block_to_free) 
    {
        prev_block->size_of_block += sizeof(Block) + block_to_free->size_of_block;
        prev_block->next_block = block_to_free->next_block;
    }
}

/*
 Deinitializes the memory manager and frees the memory pool.
 */
void mem_deinit() 
{
    free(memory_pool);  // Free the memory pool
    free(metadata_pool); // Free the metadata pool
    memory_pool = NULL; // Reset pointers
    free_list = NULL;
}
