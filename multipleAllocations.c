#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

typedef struct HEADER_TAG {
    struct HEADER_TAG *ptr_next;  
    size_t bloc_size;             
    long magic_number;           
} HEADER;

#define MAGIC_NUMBER 0x0123456789ABCDEFL
#define HEADER_SIZE sizeof(HEADER)
#define MAGIC_NUMBER_SIZE sizeof(long)

void *malloc_3is(size_t size) {
   
    size_t total_size = HEADER_SIZE + size + MAGIC_NUMBER_SIZE;
    
    void *memory_block = sbrk(total_size);
  
    HEADER *header = (HEADER*) memory_block;
    header->bloc_size = size;
    header->magic_number = MAGIC_NUMBER;
    header->ptr_next = NULL;  
        
    void *user_memory = (void*)(header + 1);  
    long *footer_magic = (long*)((char*)user_memory + size);  
    *footer_magic = MAGIC_NUMBER;
    
    return user_memory;
}

int main(){

    int *tab = (int *)malloc_3is(2 * sizeof(int)); 


    tab[0] = 10;
    tab[1] = 20; 


    printf("Valeurs du tableau :\n");
    printf("Element 0: %d\n", tab[0]);
    printf("Element 1: %d\n", tab[1]);

  
  /*  printf("Adresse du tableau: %p\n", (void *)tab); */
    
    return 0;
}

