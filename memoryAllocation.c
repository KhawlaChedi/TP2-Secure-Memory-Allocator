#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>  

typedef struct HEADER_TAG {
    struct HEADER_TAG *ptr_next;
    size_t block_size;
    long magic_number;
} HEADER;

#define HEADER_SIZE sizeof(HEADER)
#define MAGIC_NUMBER_SIZE sizeof(long)
#define MINIMUM_BLOCK_SIZE (HEADER_SIZE + 1 + MAGIC_NUMBER_SIZE)
#define PREALLOCATED_MEMORY_SIZE 1024 * 1024  // 1 Mo 

HEADER *free_list = NULL;
int memory_preallocated = 0; 
long MAGIC_NUMBER ;

long generate_random_magic_number() {
    return rand(); 
}

void sort_free_list_by_address() {
    if (!free_list || !free_list->ptr_next) return;

    HEADER *sorted = NULL;

    while (free_list) {
        HEADER *current = free_list;
        free_list = current->ptr_next;

        if (!sorted || current < sorted) {
            current->ptr_next = sorted;
            sorted = current;
        } else {
            HEADER *temp = sorted;
            while (temp->ptr_next && temp->ptr_next < current) {
                temp = temp->ptr_next;
            }
            current->ptr_next = temp->ptr_next;
            temp->ptr_next = current;
        }
    }
    free_list = sorted;
}

void merge_adjacent_blocks() {
    if (!free_list) return;

    HEADER *current = free_list;

    while (current && current->ptr_next) {
        HEADER *next_block = current->ptr_next;

        if ((char*)current + HEADER_SIZE + current->block_size + MAGIC_NUMBER_SIZE == (char*)next_block) {
            current->block_size += next_block->block_size + HEADER_SIZE + MAGIC_NUMBER_SIZE;
            current->ptr_next = next_block->ptr_next;
        } else {
            current = current->ptr_next;
        }
    }
}

void preallocate_memory() {
    if (memory_preallocated) return;

    
    srand(time(NULL)); 

    void *memory_block = sbrk(PREALLOCATED_MEMORY_SIZE);
    if (memory_block == (void *)-1) {
        perror("sbrk failed");
        exit(1);
    }

    HEADER *header = (HEADER *)memory_block;
    header->block_size = PREALLOCATED_MEMORY_SIZE - HEADER_SIZE - MAGIC_NUMBER_SIZE;
    header->magic_number = generate_random_magic_number(); 
    MAGIC_NUMBER=header->magic_number;
    header->ptr_next = NULL;

    free_list = header;
    memory_preallocated = 1;
}

HEADER *find_compatible_block(size_t size) {
    sort_free_list_by_address();  
    merge_adjacent_blocks();  

    HEADER *prev = NULL;
    HEADER *current = free_list;

    while (current) {
        if (current->block_size == size) {
            if (prev) {
                prev->ptr_next = current->ptr_next;
            } else {
                free_list = current->ptr_next;
            }
            return current;
        }

        if (current->block_size > size) {
            size_t remaining_size = current->block_size - size - HEADER_SIZE - MAGIC_NUMBER_SIZE;

            if (remaining_size >= MINIMUM_BLOCK_SIZE) {
                HEADER *new_block = (HEADER*)((char*)current + HEADER_SIZE + size + MAGIC_NUMBER_SIZE);
                new_block->block_size = remaining_size;
                new_block->magic_number = generate_random_magic_number(); 

                new_block->ptr_next = current->ptr_next;
                current->block_size = size;

                if (prev) {
                    prev->ptr_next = new_block;
                } else {
                    free_list = new_block;
                }

                return current;
            }
        }

        prev = current;
        current = current->ptr_next;
    }

    return NULL;
}

void *malloc_3is(size_t size) {
    if (!memory_preallocated) {
        preallocate_memory();  
    }

    size_t total_size = HEADER_SIZE + size + MAGIC_NUMBER_SIZE;

    HEADER *header = find_compatible_block(size);

    if (!header) {
        printf("Error: not enough preallocated memory for the request of %zu bytes.\n", size);
        return NULL;
    }

    void *user_memory = (void *)(header + 1);
    long *footer_magic = (long *)((char *)user_memory + size);
    *footer_magic = header->magic_number; 

    return user_memory;
}

int check_memory_overflow(void *base_ptr, void *element_ptr) {
    HEADER *header = (HEADER*)((char*)base_ptr - HEADER_SIZE);

    if (header->magic_number != MAGIC_NUMBER) {
        printf("Error: modification of the magic number before the header!\n");
        return -1;
    }

    long *footer_magic = (long*)((char*)base_ptr + header->block_size);
    if (*footer_magic != header->magic_number) {
        printf("Error: modification of the magic number after the block!\n");
        return -1;
    }

    char *start_address = (char *)header + HEADER_SIZE;
    char *end_address = start_address + header->block_size;

    if (element_ptr < (void *)start_address || element_ptr >= (void *)end_address) {
        printf("Error: out-of-bounds access at address %p\n", element_ptr);
        return -1;
    }

    return 0;
}

void free_3is(void *ptr) {
    if (!ptr || !memory_preallocated) return;

    HEADER *header = (HEADER*)((char*)ptr - HEADER_SIZE);

    header->ptr_next = free_list;
    free_list = header;

    sort_free_list_by_address();
    merge_adjacent_blocks();
}

int get_free_list_size() {
    int count = 0;
    HEADER *current = free_list;

    while (current) {
        count++;
        current = current->ptr_next;
    }
    return count;
}

int main() {
    int *tab = (int *)malloc_3is(2 * sizeof(int)); 
    tab[0] = 10;
    if (check_memory_overflow(tab, &tab[0]) == -1) exit(EXIT_FAILURE);
    tab[1] = 20;
    if (check_memory_overflow(tab, &tab[1]) == -1) exit(EXIT_FAILURE);
    printf("Values of the array:\n");
    printf("Element 0: %d\n", tab[0]);
    printf("Element 1: %d\n", tab[1]);
    
    /*----------------Test---------------*/
    //change magic number test
   /* tab[2] = 30; 
    if (check_memory_overflow(tab, &tab[2]) == -1) exit(EXIT_FAILURE); 
     tab[-2] = 30; 
    if (check_memory_overflow(tab, &tab[-2]) == -1) exit(EXIT_FAILURE);  */
    
    //access magic number test
   /* for (int i = 1; i >= -1; i--) {
        printf("Element %d: %d\n", i, tab[i]);
        if (check_memory_overflow(tab, &tab[i]) == -1) exit(EXIT_FAILURE);
    }*/
    /*---------------------------------*/

    free_3is(tab);
    printf("Memory freed.\n");
    printf("Size of free_list after freeing: %d\n", get_free_list_size());

    return 0;
}

