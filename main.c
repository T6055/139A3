#include <stdio.h>
#include "umem.h"
#include "umem.c"
#include <assert.h>
#include <string.h>

int main() {

    // Test umeminit, just makes the whole block 
    printf("Test Case 1: Initializing mem allocator...\n");
    if (umeminit(4096, 3) != 0) {
        printf("Failed to initialize memory\n");
        return -1;
    } 
    umemdump(); 


// Test Case 1: Initialize memory allocator with BEST_FIT algorithm
    printf("Test Case 2: Initialize mem allocator with NEXT_FIT alg\n");
    if (umeminit(4096, NEXT_FIT) != 0) {
        printf("Initialization successful!\n");
    } else {
        printf("Initialization failed!\n");
        return -1;
    }
    umemdump();

    // Allocate memory using umalloc, takes up the space s
    printf("Test Case 3: Initialize mem allocator with NEXT_FIT alg\n");
    void *ptr = umalloc(100);
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


