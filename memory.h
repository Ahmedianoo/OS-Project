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
    if (n <= 0)
        return 1;
    int power = 1;
    while (power < n)
        power *= 2;
    return power;
}

#define MIN_BLOCK_SIZE 8

static MemoryBlock *findLarger(MemoryBlock *node, int size)
{
    if (!node)
        return NULL;

    if (!node->is_split)
    {
        return (node->is_free && node->size > size) ? node : NULL;
    }
    MemoryBlock *res = findLarger(node->left, size);
    return res ? res : findLarger(node->right, size);
}

static void splitLeaf(MemoryBlock *leaf)
{
    int half = leaf->size / 2;

    leaf->left = malloc(sizeof(MemoryBlock));
    leaf->right = malloc(sizeof(MemoryBlock));

    leaf->left->start = leaf->start;
    leaf->left->size = half;
    leaf->left->is_free = true;
    leaf->left->is_split = false;
    leaf->left->left = leaf->left->right = NULL;

    leaf->right->start = leaf->start + half;
    leaf->right->size = half;
    leaf->right->is_free = true;
    leaf->right->is_split = false;
    leaf->right->left = leaf->right->right = NULL;

    leaf->is_split = true;
    leaf->is_free = false; 
}

static bool refreshFree(MemoryBlock *n)
{
    if (!n)
        return true;
    if (!n->is_split)
        return n->is_free;
    bool l = refreshFree(n->left);
    bool r = refreshFree(n->right);
    n->is_free = l && r;
    return n->is_free;
}

static MemoryBlock *findExact(MemoryBlock *n, int sz)
{
    if (!n)
        return NULL;
    if (!n->is_split)
        return (n->is_free && n->size == sz) ? n : NULL;
    MemoryBlock *r = findExact(n->left, sz);
    return r ? r : findExact(n->right, sz);
}

static void bestFitDFS(MemoryBlock *n, int sz, MemoryBlock **best)
{
    if (!n)
        return;

    if (!n->is_split)
    {
        if (n->is_free && n->size >= sz)
        {
            if (*best == NULL ||
                n->size < (*best)->size ||
                (n->size == (*best)->size && n->start > (*best)->start))
                *best = n;
        }
        return;
    }
    bestFitDFS(n->left, sz, best);
    bestFitDFS(n->right, sz, best);
}

MemoryBlock *allocateBlock(MemoryBlock *root, int request)
{
    if (!root)
        return NULL;
    int sz = nextPowerOfTwo(request);

    MemoryBlock *leaf = findExact(root, sz);
    if (leaf)
    {
        leaf->is_free = false;
        refreshFree(root);
        return leaf;
    }

    MemoryBlock *best = NULL;
    bestFitDFS(root, sz, &best);
    if (!best)
        return NULL; 

    while (best->size > sz)
    {
        if (best->size / 2 < MIN_BLOCK_SIZE)
            return NULL;
        splitLeaf(best);
        best = best->left;
    }
    best->is_free = false;
    refreshFree(root);
    return best;
}

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

bool freeBlock(MemoryBlock *node, int start)
{
    if (!node)
        return false;

    if (!node->is_split)
    {
        if (node->start == start && !node->is_free)
        {
            node->is_free = true;
            return true;
        }
        return false;
    }

    bool freed = freeBlock(node->left, start) |
                 freeBlock(node->right, start);

    if (freed &&
        node->left->is_free && node->right->is_free)
    {
        free(node->left);
        free(node->right);
        node->left = node->right = NULL;
        node->is_split = false;
        node->is_free = true;
    }
    else
        node->is_free = false;

    return freed;
}

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