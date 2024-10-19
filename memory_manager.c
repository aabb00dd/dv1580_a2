#include "memory_manager.h"

// Define the metadata structure separately from the pool
typedef struct Metadata {
    size_t size_of_block;
    int is_free;
    struct Metadata* next_block;
} Metadata;

static char *memory_pool = NULL;    // Pointer to the memory pool
static Metadata *free_list = NULL;  // Pointer to the list of free blocks
static Metadata *metadata_pool = NULL; // Separate pool for metadata

/*
 Initializes the memory manager by allocating a pool of memory.
 */
void mem_init(size_t size) 
{
    memory_pool = (char*)malloc(size); // Allocate memory for the pool
    metadata_pool = (Metadata*)malloc(sizeof(Metadata) * (size / sizeof(Metadata))); // Allocate for metadata

    if (!memory_pool || !metadata_pool) 
    {
        exit(EXIT_FAILURE);  // Exit if memory allocation fails
    }

    // Initialize the first free block
    free_list = metadata_pool; // Metadata area for the free list
    free_list->size_of_block = size;  // Entire pool size
    free_list->is_free = 1;           // Mark as free
    free_list->next_block = NULL;     // No next block
}

/*
 Allocates a block of memory of at least the specified size.
 */
void* mem_alloc(size_t size) 
{
    if (size == 0) return NULL;    // Requested size is zero

    // Pointers to traverse and track the free list
    Metadata *current_block = free_list, *prev_block = NULL;

    while (current_block) 
    {
        if (current_block->is_free && current_block->size_of_block >= size) 
        {
            if (current_block->size_of_block > size) 
            {
                Metadata *new_block = (Metadata*)((char*)memory_pool + size); // Create a new block
                new_block->size_of_block = current_block->size_of_block - size; 
                new_block->is_free = 1;                                         
                new_block->next_block = current_block->next_block;              
                current_block->size_of_block = size;                           
                current_block->next_block = new_block;
            }

            current_block->is_free = 0; 

            if (prev_block) prev_block->next_block = current_block->next_block;
            else free_list = current_block->next_block;

            return (char*)current_block + sizeof(Metadata); // Return allocated memory
        }

        prev_block = current_block;
        current_block = current_block->next_block;
    }

    return NULL;
}

/*
 Frees a previously allocated memory block.
 */
void mem_free(void* block) 
{
    if (!block) return;    

    Metadata* block_to_free = (Metadata*)((char*)block - sizeof(Metadata));

    if (block_to_free->is_free) return;

    block_to_free->is_free = 1;

    Metadata *current_block = free_list, *prev_block = NULL;

    while (current_block && current_block < block_to_free) 
    {
        prev_block = current_block;                   
        current_block = current_block->next_block;    
    }

    block_to_free->next_block = current_block;

    if (prev_block) prev_block->next_block = block_to_free;
    else free_list = block_to_free;

    if (block_to_free->next_block && 
        (char*)block_to_free + sizeof(Metadata) + block_to_free->size_of_block == (char*)block_to_free->next_block) 
    {
        block_to_free->size_of_block += sizeof(Metadata) + block_to_free->next_block->size_of_block; 
        block_to_free->next_block = block_to_free->next_block->next_block;  
    }

    if (prev_block && 
        (char*)prev_block + sizeof(Metadata) + prev_block->size_of_block == (char*)block_to_free) 
    {
        prev_block->size_of_block += sizeof(Metadata) + block_to_free->size_of_block; 
        prev_block->next_block = block_to_free->next_block;  
    }
}

/*
 Deinitializes the memory manager and frees the memory pool.
 */
void mem_deinit() 
{
    free(memory_pool);  
    free(metadata_pool); 
    memory_pool = NULL; 
    free_list = NULL;   
}
