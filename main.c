#include <stdio.h>
#include <assert.h>
#include "umem.h"
#include "umem.c"



void test_umeminit() {
    // Test umeminit, just makes the whole block 
    printf("Test Case 1: Initializing mem allocator...\n");
    if (umeminit(4096, 3) != 0) {
        printf("Failed to initialize memory\n");
        return;
    } 
    umemdump(); 
    
}

void test_first_fit() {
    // Test Case 2: Initialize memory allocator with FIRST_FITT algorithm
    printf("Test Case 2: Initialize mem allocator with FIRST_FITT...\n");
    if (umeminit(4096, FIRST_FIT) != 0) {
        printf("Initialization successful!\n");
    } else {
        printf("Test 2 Failed\n");
    }
    umemdump();
}

void test_next_fit() {
    // Test Case 3: Initialize memory allocator with NEXT_FIT algorithm
    printf("Test Case 3: Initialize mem allocator with NEXT_FIT...\n");
    if (umeminit(4096, NEXT_FIT) != 0) {
        printf("Initialization successful!\n");
    } else {
        printf("Test 3 Failed\n");
    }
    umemdump();
}

void test_umalloc() {
    // Test Case 4: Initialize mem allocator using UMALLOC...
    printf("Test Case 4: Initialize mem allocator using UMALLOC...\n");
    void *ptr = umalloc(100);
    if (ptr == NULL) {
        printf("Test 4 Failed\n");
        return;
    }
    umemdump(); 
    printf("ptr addr: %p\n", ptr);

    // Free the allocated memory chunk when done using it
    ufree(ptr);
}

int main() {
    void print_free_list(BlockF *list);
    test_umeminit();
    test_first_fit();
    test_next_fit();
    test_umalloc();

    return 0;
}

// DOES THIS DO THE SAME THING AS UMEMDUMP??
// void print_free_list(BlockF *list) {
//     printf("Current free list:\n");
//     while (list) {
//         if (list->size != 0) {
//             printf("Free block: Address=%p, Size=%zu\n",
//                    (void *)list, list->size);
//         } else {
//             printf("Error: Free block at Address=%p has invalid size\n", (void *)list);
//         }
//         list = list->next;
//     }
// }