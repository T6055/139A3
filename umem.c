#include "umem.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

typedef struct BlockF {
    size_t size;
    struct BlockF *free_next; // Pointer to the next free block
    struct BlockF *next;      // Pointer to the next block
} BlockF;

BlockF *head = NULL; // This is the head of the free list
int AllAlgo = -1;    //stores the allocation algorithm that umalloc will use. It's initialized to -1.

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
    size_t remainder = sizeOfRegion % pageSize;
    size_t allocateMem = 0; // Round up size_t to a certain page size
    if (remainder > 0)
        allocateMem = (pages + 1) * pageSize;
    else
        allocateMem = pages * pageSize;

    head = mmap(NULL, allocateMem, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    if (head == MAP_FAILED) {
        perror("mmap"); // mmap failed to get sizeOfRegion bytes
        return -1;
    }

    head->size = allocateMem - sizeof(BlockF);
    head->next = NULL;
    AllAlgo = allocationAlgo;

    return 0;
}

void *umalloc(size_t size) {
    // Implement memory allocation logic here
    // Return pointer to allocated memory or NULL if not enough space
    BlockF *prev = NULL;
    BlockF *cur = head;

    // First Fit Allocation
    while (cur != NULL) {
        if (cur->size >= size) {
            if (cur->size - size > sizeof(BlockF)) {
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
            return (void *)(cur + 1); // Pointer to the allocated memory
        }
        prev = cur;
        cur = cur->next;
    }

    // Not enough space
    return NULL;
}

int ufree(void *ptr) {
    // Implement memory deallocation logic here
    // Return 0 on success, -1 on failure
    if (ptr == NULL)
        return -1;

    BlockF *nodeToFree = (BlockF *)ptr - 1;

    // Find the correct position to insert the freed block into the free list
    BlockF *cur = head;
    BlockF *prev = NULL;
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
    BlockF *temp = head;
    while (temp != NULL) {
        if (temp->next != NULL && (char *)temp + sizeof(BlockF) + temp->size == (char *)temp->next) {
            temp->size += sizeof(BlockF) + temp->next->size;
            temp->next = temp->next->next;
        }
        temp = temp->free_next;
    }

    return 0;
}

void umemdump() {
    BlockF *cur = head;
    printf("Free Memory Dump:\n");
    while (cur != NULL) {
        printf("Address: %p, Size: %zu\n", cur, cur->size);
        cur = cur->free_next;
    }
}
