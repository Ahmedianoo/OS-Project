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

// Get the next power of two for buddy allocation
int nextPowerOfTwo(int n)
{
    if (n <= 0)
        return 1;
    int power = 1;
    while (power < n)
        power *= 2;
    return power;
}

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

// Allocate a memory block of size `size` using buddy allocation
MemoryBlock *allocateBlock(MemoryBlock *block, int size)
{
    size = nextPowerOfTwo(size);

    if (block == NULL || block->size < size)
        return NULL;

    // Only fail if leaf and not free
    if (!block->is_split && !block->is_free)
        return NULL;

    if (block->size == size && !block->is_split)
    {
        block->is_free = false;
        return block;
    }

    // Need to split if not already
    if (!block->is_split)
    {
        int half = block->size / 2;

        block->left = (MemoryBlock *)malloc(sizeof(MemoryBlock));
        block->left->start = block->start;
        block->left->size = half;
        block->left->is_free = true;
        block->left->is_split = false;
        block->left->left = NULL;
        block->left->right = NULL;

        block->right = (MemoryBlock *)malloc(sizeof(MemoryBlock));
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

    if (result != NULL)
        block->is_free = block->left->is_free && block->right->is_free;

    return result;
}

// Try to merge free buddy blocks
bool merge(MemoryBlock *block)
{
    if (!block || !block->is_split || !block->left || !block->right)
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

// Free the block starting at a given address
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

// Print the current memory layout
void printMemoryBlocks(MemoryBlock *block, int depth)
{
    if (block == NULL)
        return;

    for (int i = 0; i < depth; i++)
        printf("  ");

    printf("Block [Start: %d, Size: %d] - %s", block->start, block->size, block->is_free ? "Free" : "Allocated");

    if (block->is_split)
        printf(" (Split)\n");
    else
        printf("\n");

    printMemoryBlocks(block->left, depth + 1);
    printMemoryBlocks(block->right, depth + 1);
}

// Sample main to test allocation and deallocation
int main()
{
    char input[256];
    initializeMemory();

    // First allocation
    printf("Enter your size of mem to be allocated: ");
    fgets(input, sizeof(input), stdin);
    int size1 = atoi(input);
    MemoryBlock *mem1 = allocateBlock(memoryRoot, size1);
    if (mem1)
        printf("mem1 allocated at: %d\n", mem1->start);
    else
        printf("Allocation failed for mem1\n");

    printMemoryBlocks(memoryRoot, 0);

    // Second allocation
    printf("\nEnter your size of mem to be allocated: ");
    fgets(input, sizeof(input), stdin);
    int size2 = atoi(input);
    MemoryBlock *mem2 = allocateBlock(memoryRoot, size2);
    if (mem2)
        printf("mem2 allocated at: %d\n", mem2->start);
    else
        printf("Allocation failed for mem2\n");

    printMemoryBlocks(memoryRoot, 0);

    // Free mem1
    if (mem1)
    {
        printf("\nFreeing mem1...\n");
        if (!freeBlock(memoryRoot, mem1->start))
            printf("Failed to free mem1.\n");
        printMemoryBlocks(memoryRoot, 0);
    }

    // Free mem2
    if (mem2)
    {
        printf("\nFreeing mem2...\n");
        if (!freeBlock(memoryRoot, mem2->start))
            printf("Failed to free mem2.\n");
        printMemoryBlocks(memoryRoot, 0);
    }

    return 0;
}