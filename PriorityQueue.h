#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"

typedef struct PriorityQueueNode {
    PCB data;
    struct PriorityQueueNode* next;
} PriorityQueueNode;

typedef struct PriorityQueue {
    PriorityQueueNode* head;
} PriorityQueue;

PriorityQueue* createQueueHPF() {
    PriorityQueue* q = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (!q) {
        perror("Failed to allocate memory for queue");
        return NULL;
    }
    q->head = NULL;
    return q;
}

void enqueueHPF(PriorityQueue* q, PCB process) {
    PriorityQueueNode* newNode = (PriorityQueueNode*)malloc(sizeof(PriorityQueueNode));
    if (!newNode) {
        perror("Failed to allocate memory for node");
        return;
    }
    newNode->data = process;
    newNode->next = NULL;

    if (!q->head || process.processPriority < q->head->data.processPriority) {
        newNode->next = q->head;
        q->head = newNode;
        return;
    }

    PriorityQueueNode* current = q->head;
    while (current->next && current->next->data.processPriority <= process.processPriority) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

PCB dequeueHPF(PriorityQueue* q) {
    PCB empty = {0};
    if (!q || !q->head) {
        fprintf(stderr, "Queue underflow: No process to dequeue\n");
        return empty;
    }

    PriorityQueueNode* temp = q->head;
    PCB process = temp->data;
    q->head = temp->next;
    free(temp);
    return process;
}

PCB peekHPF(PriorityQueue* q) {
    if (!q || !q->head) {
        return (PCB){0};
    }
    return q->head->data;
}

int isEmptyHPF(PriorityQueue* q) {
    return !q || q->head == NULL;
}

void destroyQueueHPF(PriorityQueue* q) {
    PriorityQueueNode* temp;
    while (q && q->head) {
        temp = q->head;
        q->head = q->head->next;
        free(temp);
    }
    if (q) free(q);
}

#endif