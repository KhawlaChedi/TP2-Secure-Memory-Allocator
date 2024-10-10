#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct HEADER_TAG {
    struct HEADER_TAG *ptr_next;
    size_t block_size;
    long magic_number;
} HEADER;

#define MAGIC_NUMBER 0x0123456789ABCDEFL
#define HEADER_SIZE sizeof(HEADER)
#define MAGIC_NUMBER_SIZE sizeof(long)
#define MINIMUM_BLOCK_SIZE (HEADER_SIZE + 1 + MAGIC_NUMBER_SIZE)

HEADER *free_list = NULL;


void sort_free_list_by_address() {
    if (!free_list || !free_list->ptr_next) return;

    HEADER *sorted = NULL;

    while (free_list) {
        HEADER *current = free_list;
        free_list = free_list->ptr_next;

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
            current->block_size +=next_block->block_size;
            current->ptr_next = next_block->ptr_next;
        } else {
            current = current->ptr_next;
        }
    }
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
                new_block->magic_number = MAGIC_NUMBER;

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
    size_t total_size = HEADER_SIZE + size + MAGIC_NUMBER_SIZE;

    HEADER *header = find_compatible_block(size);

    if (!header) {
        void *memory_block = sbrk(total_size);

        header = (HEADER*)memory_block;
        header->block_size = size;
        header->magic_number = MAGIC_NUMBER;
        header->ptr_next = NULL;
    }

    void *user_memory = (void*)(header + 1);
    long *footer_magic = (long*)((char*)user_memory + size);
    *footer_magic = MAGIC_NUMBER;

    return user_memory;
}

int check_memory_overflow(void *base_ptr, void *element_ptr) {
    HEADER *header = (HEADER*)((char*)base_ptr - HEADER_SIZE);

    if (header->magic_number != MAGIC_NUMBER) {
        printf("Erreur : modification du magic number avant l'en-tête !\n");
        return -1;
    }

    long *footer_magic = (long*)((char*)base_ptr + header->block_size);
    if (*footer_magic != MAGIC_NUMBER) {
        printf("Erreur : modification du magic number après le bloc !\n");
        return -1;
    }

    char *start_address = (char *)header + HEADER_SIZE;
    char *end_address = start_address + header->block_size;

    if (element_ptr < (void *)start_address || element_ptr >= (void *)end_address) {
        printf("Erreur : accès hors limites à l'adresse %p\n", element_ptr);
        return -1;
    }

    return 0;
}

void free_3is(void *ptr) {
    if (!ptr) return;
    HEADER *header = (HEADER*)((char*)ptr - HEADER_SIZE);

    header->ptr_next = free_list;
    free_list = header;
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
    int *tab = (int *)malloc_3is(4 * sizeof(int));

    tab[0] = 10;
    tab[1] = 20;
    tab[2] = 30;
    tab[3] = 40;

    printf("Values of the first array:\n");
    printf("Element 0: %d\n", tab[0]);
    printf("Element 1: %d\n", tab[1]);
    printf("Element 2: %d\n", tab[2]);
    printf("Element 3: %d\n", tab[3]);

    int *new_tab = (int *)malloc_3is(6 * sizeof(int));
    new_tab[0] = 50;
    new_tab[1] = 60;

    printf("Values of the second array:\n");
    printf("Element 0: %d\n", new_tab[0]);
    printf("Element 1: %d\n", new_tab[1]);
    
    printf("\nAdresses : %p , %p\n", (void*)tab, (void*)new_tab);

    printf("Size of free_list before freeing: %d\n", get_free_list_size());
    free_3is(tab);
    free_3is(new_tab);
    printf("Memory freed.\n");
    printf("Size of free_list after freeing: %d\n", get_free_list_size());

    return 0;
}

