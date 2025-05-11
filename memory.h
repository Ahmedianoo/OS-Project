#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define TOTAL_MEMORY 1024

typedef struct MemoryBlock
{
    int start;
    int size;
    bool is_free;
    bool is_split;
    struct MemoryBlock *left;
    struct MemoryBlock *right;
} MemoryBlock;

MemoryBlock *memoryRoot;

// Initialize the root memory block
void initializeMemory()
{
    memoryRoot = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    memoryRoot->start = 0;
    memoryRoot->size = TOTAL_MEMORY;
    memoryRoot->is_free = true;
    memoryRoot->is_split = false;
    memoryRoot->left = NULL;
    memoryRoot->right = NULL;
}

int nextPowerOfTwo(int n)
{
    int power = 1;
    while (power < n)
        power *= 2;
    return power;
}

MemoryBlock *allocateBlock(MemoryBlock *block, int size)
{
    size = nextPowerOfTwo(size);

    if (!block->is_free || block->size < size)
        return NULL;

    if (block->size == size && !block->is_split)
    {
        block->is_free = false;
        return block;
    }

    if (!block->is_split)
    {
        block->left = (MemoryBlock *)malloc(sizeof(MemoryBlock));
        block->right = (MemoryBlock *)malloc(sizeof(MemoryBlock));
        int half = block->size / 2;

        block->left->start = block->start;
        block->left->size = half;
        block->left->is_free = true;
        block->left->is_split = false;
        block->left->left = NULL;
        block->left->right = NULL;

        block->right->start = block->start + half;
        block->right->size = half;
        block->right->is_free = true;
        block->right->is_split = false;
        block->right->left = NULL;
        block->right->right = NULL;

        block->is_split = true;
    }

    MemoryBlock *result = allocateBlock(block->left, size);
    if (result == NULL)
        result = allocateBlock(block->right, size);

    block->is_free = block->left->is_free && block->right->is_free;
    return result;
}

bool merge(MemoryBlock *block)
{
    if (!block->is_split || block->left == NULL || block->right == NULL)
        return false;

    if (block->left->is_free && block->right->is_free)
    {
        free(block->left);
        free(block->right);
        block->left = NULL;
        block->right = NULL;
        block->is_split = false;
        block->is_free = true;
        return true;
    }
    return false;
}

bool freeBlock(MemoryBlock *block, int start)
{
    if (block == NULL)
        return false;

    if (!block->is_split)
    {
        if (block->start == start && !block->is_free)
        {
            block->is_free = true;
            return true;
        }
        return false;
    }

    bool freed = false;
    if (block->left)
        freed |= freeBlock(block->left, start);
    if (block->right)
        freed |= freeBlock(block->right, start);

    if (freed)
        merge(block);

    if (block->left && block->right)
        block->is_free = block->left->is_free && block->right->is_free;

    return freed;
}

void logAllocation(int time, int pid, int start, int end, int size)
{
    FILE *file = fopen("memory.log", "a");
    fprintf(file, "At time %d allocated %d bytes for process %d from %d to %d\n", time, size, pid, start, end);
    fclose(file);
}

void logFree(int time, int pid, int start, int end, int size)
{
    FILE *file = fopen("memory.log", "a");
    fprintf(file, "At time %d freed %d bytes from process %d from %d to %d\n", time, size, pid, start, end);
    fclose(file);
}
void printMemoryBlocks(MemoryBlock *block, int depth)
{
    if (block == NULL)
        return;

    // Indentation based on depth in the tree
    for (int i = 0; i < depth; i++)
        printf("  ");

    printf("Block [Start: %d, Size: %d] - %s", block->start, block->size, block->is_free ? "Free" : "Allocated");

    if (block->is_split)
        printf(" (Split)\n");
    else
        printf("\n");

    // Recursive calls for children
    printMemoryBlocks(block->left, depth + 1);
    printMemoryBlocks(block->right, depth + 1);
}

// void deallocateMemory(PCB* pcb, int current_time) {
//     if (pcb->mem_block != NULL) {
//         int start = pcb->mem_block->start;
//         int size = pcb->mem_block->size;

//         // Free it from the buddy system
//         freeBlock(memoryRoot, start);

//         // Log the deallocation
//         logFree(current_time, pcb->pid, start, start + size - 1, pcb->memsize);

//         // Nullify pointer
//         pcb->mem_block = NULL;
//     }
// }

int main(int argc, char *argv[])
{
    char message[256];

    printf("Enter your size of mem to be allocated: ");
    if (fgets(message, sizeof(message), stdin) != NULL)
    {
        printf("You entered: %s", message);
    }
    else
    {
        printf("Error reading input.\n");
    }

    initializeMemory();
    MemoryBlock *mem = allocateBlock(memoryRoot, atoi(message));
    printf("\n%d\n", mem->start);
    printf("Current Memory Layout:\n");
    printMemoryBlocks(memoryRoot, 0);
    freeBlock(memoryRoot, mem->start);
    printf("Current Memory Layout:\n");
    printMemoryBlocks(memoryRoot, 0);

    // logAllocation();
}