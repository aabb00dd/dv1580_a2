#include "memory_manager.h"

static char *memory_pool = NULL;    
static char *header_pool = NULL;      
static Block *free_list = NULL;      
static pthread_mutex_t memory_lock = PTHREAD_MUTEX_INITIALIZER;

void mem_init(size_t size) 
{
    memory_pool = (char*)malloc(size); 
    header_pool = (char*)malloc(size * 10000); 

    if (!memory_pool || !header_pool) 
    {
        exit(EXIT_FAILURE);
    }

    free_list = (Block*)header_pool;         
    free_list->size_of_block = size;      
    free_list->is_free = 1;              
    free_list->next_block = NULL;            
    free_list->data = memory_pool;     
}

void* mem_alloc(size_t size) 
{
    pthread_mutex_lock(&memory_lock);

    if (!memory_pool || !header_pool)
    {
        pthread_mutex_unlock(&memory_lock);
        return NULL;
    }

    if (size == 0)
    {
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
        pthread_mutex_unlock(&memory_lock);
        return NULL;
    }

    Block *current_block = free_list, *prev_block = NULL;

    while (current_block) 
    {
        if (current_block->is_free && current_block->size_of_block >= size) 
        {
            if (current_block->size_of_block > size + sizeof(Block)) 
            {
                Block *new_block = (Block*)((char*)current_block + sizeof(Block));
                new_block->size_of_block = current_block->size_of_block - size;        
                new_block->is_free = 1;                                            
                new_block->next_block = current_block->next_block;               
                new_block->data = current_block->data + size;                  
                
                current_block->size_of_block = size;                 
                current_block->next_block = new_block;                     
            }

            current_block->is_free = 0;

            if (prev_block)
                prev_block->next_block = current_block->next_block;
            else
                free_list = current_block->next_block;

            pthread_mutex_unlock(&memory_lock);
            return current_block->data;
        }

        prev_block = current_block;
        current_block = current_block->next_block;
    }

    pthread_mutex_unlock(&memory_lock);
    return NULL;
}


void mem_free(void* block) 
{
    pthread_mutex_lock(&memory_lock);

    if (!block || !memory_pool || !header_pool) {
        pthread_mutex_unlock(&memory_lock);
        return;
    }

    Block *block_to_free = (Block*)((char*)block - sizeof(Block));

    if ((char*)block_to_free < header_pool || (char*)block_to_free >= header_pool + free_list->size_of_block) {
        pthread_mutex_unlock(&memory_lock);
        return;
    }

    if (block_to_free->is_free) {
        pthread_mutex_unlock(&memory_lock);
        return;
    }

    block_to_free->is_free = 1;

    if (block_to_free->next_block && 
        (char*)block_to_free->next_block < header_pool + free_list->size_of_block &&
        block_to_free->next_block->is_free) 
    {
        block_to_free->size_of_block += sizeof(Block) + block_to_free->next_block->size_of_block;
        block_to_free->next_block = block_to_free->next_block->next_block;
    }

    Block *current = free_list;
    Block *prev = NULL;

    while (current && current < block_to_free) {
        prev = current;
        current = current->next_block;
    }

    if (prev) {
        if ((char*)prev + sizeof(Block) + prev->size_of_block == (char*)block_to_free) {
            prev->size_of_block += sizeof(Block) + block_to_free->size_of_block;
            prev->next_block = block_to_free->next_block;
        } else {
            block_to_free->next_block = prev->next_block;
            prev->next_block = block_to_free;
        }
    } else {
        block_to_free->next_block = free_list;
        free_list = block_to_free;
    }

    pthread_mutex_unlock(&memory_lock);
}


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

    Block* old_block = (Block*)((char*)block - sizeof(Block));

    if (old_block->size_of_block >= size) {
        pthread_mutex_unlock(&memory_lock);
        return block;
    }

    void* new_block = mem_alloc(size);

    if (new_block) 
    {
        memcpy(new_block, block, old_block->size_of_block); 
        mem_free(block);                                  
    }
    
    pthread_mutex_unlock(&memory_lock);
    return new_block;
}


void mem_deinit() 
{
    pthread_mutex_lock(&memory_lock); 
    free(memory_pool); 
    free(header_pool); 
    memory_pool = NULL; 
    header_pool = NULL; 
    free_list = NULL;   

    pthread_mutex_unlock(&memory_lock); 
    pthread_mutex_destroy(&memory_lock); 
}
