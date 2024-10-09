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

HEADER *free_list = NULL;  

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

int main(){

   
    int *tab = (int *)malloc_3is(2 * sizeof(int)); 
 /* printf("Adresse du tableau: %p\n", (void *)tab); */

    tab[0] = 10;
    tab[1] = 20; 

    printf("Valeurs du tableau :\n");
    printf("Element 0: %d\n", tab[0]);
    printf("Element 1: %d\n", tab[1]);
    
    printf("Taille de free_list avant libération: %d\n", get_free_list_size());

    free_3is(tab);
    printf("Mémoire libérée.\n");

    printf("Taille de free_list après libération: %d\n", get_free_list_size());
    
    return 0;
}

