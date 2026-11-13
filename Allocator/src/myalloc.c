#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "myalloc.h"

#define MEMORY_SIZE     (1024)
#define MIN_BLOCK_SIZE  (8)


/*
 * Define free memory block linked list's nodes.
 * Using the address of the block itself as the
 * memory's address in the memory pool
 */
typedef struct mem_block_t
{
    struct mem_block_t *next;
    uint32_t size;
} mem_block_t;


/* Create memory pool */
uint32_t memory[MEMORY_SIZE];

mem_block_t *free_head = NULL;
static bool initialized = false;


/**
 * Allocates a block of memory. Returns a pointer to the start
 * of the allocated memory. Returns NULL if no memory is available.
 * @param size The size of the memory block to allocate.
 * @returns A pointer to the start of the allocated memory block
 * or NULL if no memory is available
 */
void *
myalloc(int size)
{
    mem_block_t *previous = NULL;
    uint32_t block_size;
    mem_block_t *remainder;

    /* Initialize the free list on the first call */
    if (!initialized)
    {
        free_head = (mem_block_t *)memory;
        free_head->size = sizeof(memory);
        free_head->next = NULL;
        initialized = true;
    }
    mem_block_t *current = free_head;

    if (size <= 0)
    {
        return NULL;
    }

    /* Ensure the requested size is at least the minimum block size */
    if (size < MIN_BLOCK_SIZE)
    {
        size = MIN_BLOCK_SIZE;
    }
    /* Round up to the nearest 4 byte aligned value */
    size = ((size + 3) / 4) * 4;
    block_size = sizeof(mem_block_t) + size;

    while (current != NULL)
    {
        if (current->size >= block_size)
        {
            /* Split the block if the remainder can hold a new block */
            if (current->size >= block_size + sizeof(mem_block_t) + MIN_BLOCK_SIZE)
            {
                remainder = (mem_block_t *)((uint8_t *)current + block_size);
                remainder->size = current->size - block_size;
                remainder->next = current->next;

                if (previous == NULL)
                {
                    free_head = remainder;
                }
                else
                {
                    previous->next = remainder;
                }

                current->size = block_size;
            }
            else
            {
                /* Use the entire free block */
                if (previous == NULL)
                {
                    free_head = current->next;
                }
                else
                {
                    previous->next = current->next;
                }
            }

            current->next = NULL;
            return (void *)(current + 1);
        }

        previous = current;
        current = current->next;
    }

    return NULL;
}


/**
 * Frees a block of allocated memory. The function assumes the
 * pointer is valid and was allocated by myalloc. It does not
 * check if the pointer is valid or if it was allocated by myalloc.
 * @param start A pointer to the start of the memory block to free
 */
void
myfree(void *start)
{
    mem_block_t *block;
    mem_block_t *previous = NULL;
    mem_block_t *current = free_head;

    if (start == NULL)
    {
        return;
    }

    block = ((mem_block_t *)start - 1);
    /* Find where the block should be inserted */
    while (current != NULL && current < block)
    {
        previous = current;
        current = current->next;
    }

    block->next = current;
    if (previous == NULL)
    {
        free_head = block;
    }
    else
    {
        previous->next = block;
    }

    /* Merge with the next block if they are adjacent */
    if (block->next != NULL && ((uint8_t *)block + block->size == (uint8_t *)block->next))
    {
        block->size += block->next->size;
        block->next = block->next->next;
    }
    /* Merge with the previous block if they are adjacent */
    if (previous != NULL && ((uint8_t *)previous + previous->size == (uint8_t *)block))
    {
        previous->size += block->size;
        previous->next = block->next;
    }

    return;
}


/**
 * Returns a string describing the free memory blocks.
 * @returns A string describing the free memory blocks
 * or NULL if there are no more blocks.
 */
char *
print_status(void)
{
    static char str[128];
    static mem_block_t *status_current = NULL;
    static bool status_started = false;

    if (!status_started)
    {
        status_current = free_head;
        status_started = true;
    }
    if (status_current == NULL)
    {
        status_started = false;
        return NULL;
    }

    sprintf(str, "Free block at address: 0x%08lX, size: %lu bytes\n", (uint32_t)(status_current - 1), status_current->size);
    status_current = status_current->next;
    return str;
}


/*
 * On memory leaks. Assuming this would be in an embedded context:
 * My approach to detecting memory leaks would be to monitor the total
 * amount of allocated memory. If the amount of allocated memory keeps
 * increasing in a detectable pattern the allocator would flag it as a
 * memory leak.
 *
 * The allocater would keep a history of the total amount of allocated memory
 * for some fixed max period or number of allocations. The allocator would check
 * if the max total allocated memory is continually increasing and then
 * check if the allocations follow a pattern with some kind of algorithm.
 *
 * This would detect one kind of leak where the program using the allocator
 * has a loop or a regular process that allocates memory but forgets to
 * free some of it. The total allocated memory would keep peaking every iteration
 * of the loop or process.
 *
 * This approach would not detect other memory leaks where the program allocates
 * a block of memory once and then forgets to free it or where a program loses
 * access to an allocated block of memory.
 *
 * I don't really know how you would actually go about implementing this, the
 * pattern detection especially, but the idea seems alright to me.
 */