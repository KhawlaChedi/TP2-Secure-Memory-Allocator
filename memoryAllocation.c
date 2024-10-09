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

HEADER *free_list = NULL;  

void *malloc_3is(size_t size) {
    size_t total_size = HEADER_SIZE + size + MAGIC_NUMBER_SIZE;
    
    void *memory_block = sbrk(total_size);
    
    HEADER *header = (HEADER*) memory_block;
    header->block_size = size;
    header->magic_number = MAGIC_NUMBER;
    header->ptr_next = NULL;  
        
    void *user_memory = (void*)(header + 1);  
    long *footer_magic = (long*)((char*)user_memory + size);  
    *footer_magic = MAGIC_NUMBER;
    
    return user_memory;
}

int check_memory_overflow(void *base_ptr, void *element_ptr) {

    HEADER *header = (HEADER*)((char*)base_ptr - HEADER_SIZE);   
    if (header->magic_number != MAGIC_NUMBER) {
        printf("Error: magic number modified before the bloc!\n");
        return -1;
    }

   
    long *footer_magic = (long*)((char*)base_ptr + header->block_size);
    if (*footer_magic != MAGIC_NUMBER) {
        printf("Error: magic number modified after the block!\n");
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
    int *tab = (int *)malloc_3is(2 * sizeof(int)); 

    tab[0] = 10;
    if (check_memory_overflow(tab, &tab[0]) == -1) exit(EXIT_FAILURE);
    tab[1] = 20;
    if (check_memory_overflow(tab, &tab[1]) == -1) exit(EXIT_FAILURE);
    
    printf("Values of the array:\n");
    printf("Element 0: %d\n", tab[0]);
    printf("Element 1: %d\n", tab[1]);
    
    //change magic number test
    tab[2] = 30; 
    if (check_memory_overflow(tab, &tab[2]) == -1) exit(EXIT_FAILURE); 
     tab[-2] = 30; 
    if (check_memory_overflow(tab, &tab[-2]) == -1) exit(EXIT_FAILURE);  
    
    //access test
    for (int i = 1; i >= -1; i--) {
        printf("Element %d: %d\n", i, tab[i]);
        if (check_memory_overflow(tab, &tab[i]) == -1) exit(EXIT_FAILURE);
    }

    printf("Size of free_list before freeing: %d\n", get_free_list_size());

    free_3is(tab);
    printf("Memory freed.\n");

    printf("Size of free_list after freeing: %d\n", get_free_list_size());
    
    return 0;
}

