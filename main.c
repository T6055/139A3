#include <stdio.h>
#include "umem.h"
#include "umem.c"
#include <assert.h>
#include <string.h>

int main() {

    // Test umeminit, just makes the whole block 
    if (umeminit(4096, 3) != 0) {
        printf("Failed to initialize memory\n");
        return -1
        ;
    } 

    umemdump(); 

    // Allocate memory using umalloc, takes up the space s
    void* ptr = umalloc(100);
    if (ptr == NULL) {
        printf("Failed to allocate memory\n");
        return -1;
    }

       umemdump(); 
       printf("ptr addr: %p\n", ptr);

    // Free the allocated memory chunk when done using it
    ufree(ptr);

    return 0;



}


