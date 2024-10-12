# Secure Memory Allocator – TP 2

## Description

This project implements a custom memory allocator that replaces standard allocation functions like `malloc`, `free`, etc., to manage memory and control buffer overflows. The allocator monitors and reports any memory corruption due to buffer overflow during write operations.

### Objectives of the TP

1. **Memory Block Management**: The allocator manages memory using a linked list of free memory blocks. When a block is freed, it is merged with adjacent blocks to avoid memory fragmentation.

2. **Overflow Control**: Each allocated memory block is surrounded by "magic numbers" to detect and report unauthorized memory modifications. If these values change, the allocator flags an error.

3. **Memory Preallocation**: An initial amount of memory is preallocated to handle future allocations efficiently. If the space runs out, additional memory is allocated using the `sbrk` function.

## Features

- **Memory Allocation with `malloc_3is`**: The allocator searches for a memory block of sufficient size and splits it if necessary. If no suitable block is available, additional memory is allocated.
  
- **Memory Deallocation with `free_3is`**: Freed blocks are reintegrated into the list of available blocks and merged with adjacent blocks if possible.

- **Overflow Detection**: The allocator uses "magic numbers" placed before and after each memory block to detect buffer overflows during read or write operations.

- **Adjacent Block Merging**: Free adjacent memory blocks are automatically merged to prevent excessive memory fragmentation.

## Code Structure

- `HEADER_TAG`: Structure for managing free memory blocks, containing:
  - A pointer to the next free block (`ptr_next`).
  - The size of the block in bytes (`block_size`).
  - A "magic number" to detect memory overflows.

- `malloc_3is(size_t size)`: Allocation function that returns a memory block of the given `size`.
- `free_3is(void *ptr)`: Frees a previously allocated memory block.
- `check_memory_overflow(void *base_ptr, void *element_ptr)`: Verifies memory overflows by checking the "magic numbers."
- `preallocate_memory()`: Initializes a predefined memory block using the `sbrk` function.

## Tests

The program includes basic tests to validate the following functionalities:
1. **Memory Allocation and Deallocation**: Allocates an array of two integers, checks for overflows, and frees the memory.
2. **Overflow Detection**: Tests the modification and verification of "magic numbers."
3. **Block Merging**: After memory is freed, the program checks the merging of adjacent free blocks.
