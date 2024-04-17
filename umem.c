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

void* BEST_AFIT(size_t size) {
  void* ptr = NULL;

  return ptr;
} 

void* WORST_AFIT(size_t size) {
  void* ptr = NULL;
  return ptr;
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
            }

            if (prev == NULL)
                head = cur->next;
            else
                prev->next = cur->next;

            //umemdump();

            // void *ptr = NULL;
            return (void *)(cur + 1); // Pointer to the allocated memory
        }
        prev = cur;
        cur = cur->next;
    }

    // Not enough space
    return NULL;
} 

void* NEXT_AFIT(size_t size) {
    if (head == NULL) {
    }

    BlockF *cur = head;
    while (1) {
        if (cur->size >= size) {
            head = cur;
            return (void *)(cur + 1);
        }
        cur = cur->next ? cur->next : head;
        
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
    AllAlgo = allocationAlgo;

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


int ufree(void *ptr) {
    BlockF *prev = NULL;
    BlockF *nodeToFree = NULL;
    BlockF *temp = NULL; 
    
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
            nodeToFree->free_next = head;
            head = nodeToFree;
        } else {
            // Insert in between or at the end
            nodeToFree->free_next = prev->free_next;
            prev->free_next = nodeToFree;
        }

        // Merge contiguous free blocks
        temp = (BlockF*)((char*)head);
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
        }
    }

    // Coalesce with previous block if free
    prev = head;
    temp = head;
    while (temp && temp->free_next) {
        prev = prev->free_next;
    }
    if (prev) {
        BlockF *nextBlock = temp->next;
        if (nextBlock->free_next == nodeToFree->free_next) {
            prev->size += sizeof(BlockF) + nodeToFree->size;
            prev->free_next = nodeToFree->free_next;
        }
    }
    return 0;
}


    

void umemdump() {
    BlockF *cur = head;
    printf("Free Memory Dump:\n");
    //htpr = (header)
    while (cur != NULL) {
        printf("Address: %p, Size: %zu\n", cur, cur->size);
        cur = cur->free_next;
    }
}

