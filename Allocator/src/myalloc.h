#pragma once

/**
 * Allocates a block of memory. Returns a pointer to the
 * start of the allocated memory. Returns NULL if no memory
 * is available.
 * @param size The size of the memory block to allocate.
 * @returns A pointer to the start of the allocated memory
 * block or NULL if no memory is available
 */
void *
myalloc(int);

/**
 * Frees a block of allocated memory. The function does not yet
 * check if the pointer is valid or if it was allocated by myalloc.
 * @param start A pointer to the start of the memory block to free
 */
void
myfree(void *);

/**
 * Returns a string describing the free memory blocks.
 * @returns A string describing the free memory blocks
 * or NULL if there are no more blocks.
 */
char *
print_status(void);