#include "memory_manager.h"

static char *memory_pool = NULL;    // Pointer to the memory pool
static Block *free_list = NULL;     // Pointer to the list of free blocks

// Align size to a multiple of alignment bytes (e.g., 8 or 16)
#define ALIGN(size) (((size) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1))

void mem_init(size_t size) 
{
    size = ALIGN(size);   // Ensure the total memory pool is aligned
    memory_pool = (char*)malloc(size);

    if (!memory_pool) 
    {
        exit(EXIT_FAILURE);
    }

    // Initialize the first block to the entire pool
    free_list = (Block*)memory_pool;
    free_list->size_of_block = size - sizeof(Block); // Store block size, excluding header
    free_list->is_free = 1;
    free_list->next_block = NULL;
}

void* mem_alloc(size_t size) 
{
    if (size == 0) return NULL;

    size = ALIGN(size);   // Align the requested size

    Block *current_block = free_list, *prev_block = NULL;

    while (current_block) 
    {
        if (current_block->is_free && current_block->size_of_block >= size) 
        {
            if (current_block->size_of_block >= size + 8)  // Split only if the remaining space is large enough
            {
                // Create a new block after the allocated block
                Block *new_block = (Block*)((char*)current_block + sizeof(Block) + size);
                new_block->size_of_block = current_block->size_of_block - size;
                new_block->is_free = 1;
                new_block->next_block = current_block->next_block;

                // Update current block
                current_block->size_of_block = size;
                current_block->next_block = new_block;
            }

            current_block->is_free = 0; // Mark as allocated
            return (char*)current_block + sizeof(Block); // Return pointer to memory after block header
        }

        prev_block = current_block;
        current_block = current_block->next_block;
    }

    return NULL; // No suitable block found
}

void mem_free(void* block) 
{
    if (!block) return;

    Block* block_to_free = (Block*)((char*)block - sizeof(Block));

    if (block_to_free->is_free) return;

    block_to_free->is_free = 1;

    Block *current_block = free_list;

    // Try to merge with next block
    if (block_to_free->next_block && block_to_free->next_block->is_free)
    {
        block_to_free->size_of_block += sizeof(Block) + block_to_free->next_block->size_of_block;
        block_to_free->next_block = block_to_free->next_block->next_block;
    }

    // Check if the free block is before the free_list and update free_list
    if (block_to_free < free_list) 
    {
        block_to_free->next_block = free_list;
        free_list = block_to_free;
    } 
    else 
    {
        // Traverse free list to insert the freed block
        while (current_block && current_block->next_block < block_to_free)
        {
            current_block = current_block->next_block;
        }

        // Insert into the free list
        block_to_free->next_block = current_block->next_block;
        current_block->next_block = block_to_free;
    }

    // Merge with the previous block if they are contiguous
    if (current_block && current_block->is_free && 
        (char*)current_block + sizeof(Block) + current_block->size_of_block == (char*)block_to_free)
    {
        current_block->size_of_block += sizeof(Block) + block_to_free->size_of_block;
        current_block->next_block = block_to_free->next_block;
    }
}

void* mem_resize(void* block, size_t size) 
{
    if (!block) return mem_alloc(size);

    size = ALIGN(size);

    Block* old_block = (Block*)((char*)block - sizeof(Block));
    if (old_block->size_of_block >= size) return block;

    void* new_block = mem_alloc(size);

    if (new_block) 
    {
        memcpy(new_block, block, old_block->size_of_block);
        mem_free(block);
    }

    return new_block;
}

void mem_deinit() 
{
    free(memory_pool);
    memory_pool = NULL;
    free_list = NULL;
}
