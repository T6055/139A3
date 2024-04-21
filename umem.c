#include "umem.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>  

typedef struct BlockF {
    size_t size;
    struct BlockF *free_next; // Pointer to the next free block
    struct BlockF *next;      // Pointer to the next block
} BlockF;

BlockF *head = NULL; // This is the head of the free list
int AllAlgo = -1;    //stores the allocation algorithm that umalloc will use. It's initialized to -1.
//BlockF *head = NULL;

void Print(){
    BlockF* temp = head;
    printf("\n\nPRINT:\n");
    while(temp){
        printf("%p\n\t%p\n", temp, temp->free_next);
        temp = temp->next;
    }
}

void* BEST_AFIT(size_t size) {
  BlockF *cur = head;
    BlockF *best = NULL;

    while (cur != NULL) {
        if (cur->size >= size && (best == NULL || cur->size < best->size)) {
            best = cur;
        }
        cur = cur->next;
    }
    return best;
} 

void* WORST_AFIT(size_t size) {
    BlockF *cur = head;
    BlockF *worst = NULL;

    // Find the worst-fit block
    while (cur != NULL) {
        if (cur->size >= size) {
            if (worst == NULL || cur->size > worst->size) {
                worst = cur;
            }
        }
        cur = cur->next;
    }
    // If a worst-fit block is found, allocate memory from it
    if (worst != NULL) {
        // If the worst-fit block is exactly the same size as requested, remove it from the free list
        if (worst->size == size) {
            // Update the head if necessary
            if (worst == head) {
                head = worst->next;
            }
            return worst + 1; // Return a pointer to the allocated memory
        } else if (worst->size > size) {
            // If the worst-fit block is larger, split it and allocate from the beginning
            BlockF *cur = worst;
            BlockF *newNode = (BlockF *)((char *)cur + size + sizeof(BlockF));
            newNode->size = cur->size - size - sizeof(BlockF);
            newNode->next = cur->next;
            if (worst == head) {
                head = newNode;
            }
            return cur + 1; // Return a pointer to the allocated memory
        }
    }
    // Return NULL if no suitable block is found
    return NULL;
}

void* FIRST_AFIT(size_t size) {
    if (size <= 0)
        return NULL;

    BlockF *prev = NULL;
    BlockF *cur = head;
    // Traverse the free list to find a good block
    while (cur != NULL) {
        if (cur->size >= size) {
            // Allocate from the current block
            if (cur->size - size > sizeof(BlockF)) {
                // Split the block if it's larger than needed
                BlockF *newNode = (BlockF *)((char *)cur + sizeof(BlockF) + size);
                newNode->size = cur->size - sizeof(BlockF) - size;
                newNode->free_next = cur->free_next;
                newNode->next = cur->next;
                cur->size = size;
                cur->free_next = NULL;
                cur->next = newNode;
                //printf("newNode: %p\n", newNode);
            }

            if (prev == NULL)
                head = cur->next;
            else
                prev->next = cur->next;
            printf("alloc: %p\n", cur);
            return (void *)(cur + 1); // Pointer to the allocated memory
        }
        prev = cur;
        cur = cur->next;
    }
    // Not enough space
    return NULL;
} 

void* NEXT_AFIT(size_t size) {
    if (head == NULL) { // If the free list is empty, there are no blocks to allocate, so return NULL
        return NULL; 

    }
    BlockF *cur = head; // Start the search from the current head of the free list
    while (1) {
        if (cur->size >= size) { // If the current block's size is ok to hold the requested sizeallocate from it
            head = cur; // Update the head of the free list to the current block
            printf("next alloc: %p\n", cur);
            return (void *)(cur + 1); // Return a pointer to the allocated memory
        }
        cur = cur->next ? cur->next : head; 
        // Move to the next block in the free list, or loop back to the head if we reach the end  
    }
    return NULL;
}

int umeminit(size_t sizeOfRegion, int allocationAlgo) {
    if (head != NULL || sizeOfRegion <= 0) {
        return -1;
    }

    int PageSize = getpagesize();
    if (PageSize < 0) {
        perror("getpagesize");
        return -1;
    }

    size_t pageSize = (size_t)PageSize; // Computer's page size
    size_t pages = sizeOfRegion / pageSize;
    size_t tSize = sizeOfRegion % pageSize;
    size_t umalMem = 0; // Round up size_t to a certain page size
    if (tSize > 0)
        umalMem = (pages + 1) * pageSize;
    else
        umalMem = pages * pageSize;

    head = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (head == MAP_FAILED) {
        perror("mmap"); // mmap failed to get sizeOfRegion bytes
        return -1;
    }

    head->size = umalMem - sizeof(BlockF);
    head->next = NULL;
    head->free_next = NULL;
    AllAlgo = allocationAlgo;
    printf("umeminit head: %p\n\tsize: %ld\n", head, head->size);
    return 0;
}

void *umalloc(size_t size) {
    size_t bytes = size / 8;
    size_t tSize = size % 8;
    size_t umalMem = 0; // Round up size_t to a certain number of 8 bytes
    if (tSize > 0)
        umalMem = (bytes + 1) * 8;
    else
        umalMem = bytes * 8;

     void *block = NULL;
            
            switch (AllAlgo){
                case BEST_FIT:
                    block = BEST_AFIT(umalMem);
                    break;
                case WORST_FIT:
                    block = WORST_AFIT(umalMem);
                    break;
                case FIRST_FIT:
                    block = FIRST_AFIT(umalMem);
                    break;
                case NEXT_FIT:
                    block = NEXT_AFIT(umalMem);
                    break; 
                default:
                    perror("AllAlgo");
                    return NULL; 
            }

    return block;
}

int ufree(void *ptr) { //THIS IS NOT COALESING 
    BlockF *prev = NULL;
    BlockF *nodeToFree = NULL;
    //BlockF *temp = NULL; 

    if (ptr != NULL) {
        nodeToFree = (BlockF *)ptr - 1;
        // Find the correct position to insert the freed block into the free list
        BlockF *cur = head;
        prev = NULL;
        while (cur != NULL && cur < nodeToFree) {
            prev = cur;
            cur = cur->free_next;
        }

        if (prev == NULL) {
            // Insert at the beginning
            nodeToFree->free_next = head->free_next;
            head = nodeToFree;
        } else {
            // Insert in between or at the end
            nodeToFree->free_next = prev->free_next;
            prev->free_next = nodeToFree;
            //
            //
            //
            //
            //
            //

            if(prev->next == nodeToFree){
                // coalesce block before
                prev->size += nodeToFree->size;
                prev->free_next = nodeToFree->free_next;
                prev->next = prev->next->next;
                nodeToFree = prev;
            }
            //
            //
            //
            //
            //

        }
        //printf("head in ufree is: %p\n", head);

        //
        //
        //
        //
        //
        //
        if(nodeToFree->next == nodeToFree->free_next){
            //coalesce block after
            nodeToFree->size += nodeToFree->free_next->size;
            nodeToFree->next = nodeToFree->next->next;
            nodeToFree->free_next = nodeToFree->free_next->free_next;
            
        }
        //
        //
        //
        //
        //
        //
        // Merge contiguous free blocks
        /*temp = (BlockF*)((char*)head);
        while (temp != NULL) {
            BlockF *nextBlock = temp->free_next;
            if (nextBlock == head->next) { // doesnt matter if the next head is free 
                temp->size += sizeof(BlockF) + nextBlock->size;
                temp->free_next = nextBlock->free_next;
            } else {
                temp = temp->next;
            }
        }

        //merge previous block nextblock == next free block 
        temp = (BlockF*)((char*)head);
        prev = NULL;
        while (temp && temp->free_next) {
            BlockF *nextBlock = temp->free_next; // comes from the free lists so we know its free  
            if (nextBlock-> next == head) { // doesnt matter if the next head is free 
                temp->size += sizeof(BlockF) + nextBlock->size;
                prev->free_next = nextBlock->free_next;
            } else {
                prev =temp;
                temp = temp->next;
            }
        }*/
    }

    /*// Coalesce with previous block if free
    prev = head;
    temp = head;
    while (temp && temp->free_next) {
        if(prev->free_next)
            prev = prev->free_next;
    }
    if (prev) {
        BlockF *nextBlock = temp->next;
        if (nextBlock->free_next == nodeToFree->free_next) {
            prev->size += sizeof(BlockF) + nodeToFree->size;
            prev->free_next = nodeToFree->free_next;
        }
    }*/
    return 0;
}

void umemdump() {
    BlockF *cur = head;
    printf("Free Memory Dump:\n");
    //htpr = (header)
    while (cur != NULL) {
        printf("Address: %p, Size: %zu next: %p\n", cur, cur->size, cur->free_next);
        cur = cur->free_next;
    }
}