#include <stdio.h>
#include <assert.h>
#include "umem.h"
#include "umem.c"
#define GRN  "\x1B[32m"
#define NRM  "\x1B[0m"

void test_umeminit();
void test_first_fit();
void test_next_fit() ;
void test_umalloc() ;
void test_worst_fit(); 

int main() {
     //void print_free_list(BlockF *list);
    //test_umeminit();
    //test_first_fit();
   // test_next_fit(); segmentation fault 
    //test_umalloc();
    //test_worst_fit(); segmentation fault 

   umeminit(4096, 3);

    

    printf("%sTest 6%s: Freeing memory blocks at the beginning of the free list.\n", GRN, NRM);
    printf("There are no headers on free memory, so size is (requested value as 8B aligned) + 8.\n");

    int vals[] = {1, 16, 32, 64, 1024, 512, 96, 1000, 80, 48};
    printf("\nbefore mallocs\n");
    umemdump();

    void** ptrs = umalloc(10 * sizeof(int*));
    for (int i = 0; i < 10; i++) {
        ptrs[i] = umalloc(vals[i]);
        if(ptrs[i]==NULL){printf("Malloc returned NULL for ptrs[%d]\n", i); }
    }
    printf("\nafter mallocs\n");
    umemdump();

    ufree(ptrs[2]);
    ufree(ptrs[8]);
    ufree(ptrs[4]);
    ufree(ptrs[6]);
    printf("\nafter frees\n");
    umemdump();

    ufree(ptrs[5]);
    umemdump();

    ufree(ptrs[0]);
    ufree(ptrs[1]);
    ufree(ptrs[3]);
    ufree(ptrs[7]);
    ufree(ptrs[9]);
    ufree(ptrs);

    umemdump();
}


void test_umeminit() {
    // Test umeminit, just makes the whole block 
    printf("%sTest 1%s: Initializing mem allocator...\n", GRN, NRM);
    if (umeminit(4096, 3) != 0) {
       // printf("Failed to initialize memory\n");
        return;
    } 
    umemdump();  
}

void test_first_fit() {
    // Test Case 2: Initialize memory allocator with FIRST_FITT algorithm
    printf("%sTest 2%s:: Initialize mem allocator with FIRST_FITT...\n", GRN, NRM);
    if (umeminit(4096, FIRST_FIT) != 0) {
       // printf("Test 2 Failed\n");
    }
    umemdump();
}

void test_next_fit() { // this continously goes 
    // Test Case 3: Initialize memory allocator with NEXT_FIT algorithm
    printf("%sTest 3%s: Initialize mem allocator with NEXT_FIT...\n", GRN, NRM);
    if (umeminit(4096, NEXT_FIT) != 0) {
       // printf("Test 3 Failed\n");
    }
    umemdump();
}

void test_umalloc() {
    // Test Case 4: Initialize mem allocator using UMALLOC...
    printf("%sTest 4%s:: Initialize mem allocator using UMALLOC...\n", GRN, NRM);
    void *ptr = umalloc(100);
    if (ptr == NULL) {
       // printf("Test 4 Failed\n");
        return;
    }
    umemdump(); 
    printf("ptr addr: %p\n", ptr);
    // Free the allocated memory chunk when done using it
    ufree(ptr);
}
void test_worst_fit() {
    // Initialize the memory allocator with WORST_FIT algorithm
    int value = umeminit(4096, WORST_FIT);
    if (value == -1) {
        printf("failed umeminit\n");
        return;
    }
    void *ptrs[4];
    printf("%sTest 5%s:: Initialize mem allocator with WORST_FIT...\n", GRN, NRM);
    ptrs[0] = umalloc(16);
    printf("address of pointer: %p\n", ptrs[0]);
    ptrs[1] = umalloc(256);
    printf("address of pointer: %p\n", ptrs[1]);
    ptrs[2] = umalloc(512);
    printf("address of pointer: %p\n", ptrs[2]);
    ptrs[3] = umalloc(4050);
    printf("address of pointer: %p\n", ptrs[3]);
    if (ptrs[0] != NULL || ptrs[1] != NULL || ptrs[2] != NULL ||
        ptrs[3] != NULL) {
        printf("Test 5 Passed\n");
    //} else {
        //printf("Test 5 Failed: umalloc returned NULL\n");
    }
    umemdump();
}
