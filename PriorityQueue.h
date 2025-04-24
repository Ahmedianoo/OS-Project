#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"

typedef struct Node {
    PCB data;
    struct Node* next;
} Node;

typedef struct PriorityQueue {
    Node* head;
} PriorityQueue;

PriorityQueue* createQueue() {
    PriorityQueue* q = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (!q) {
        perror("Failed to allocate memory for queue");
        return NULL;
    }
    q->head = NULL;
    return q;
}

void enqueue(PriorityQueue* q, PCB process) {
    Node* newNode = (Node*)malloc(sizeof(Node));
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

    Node* current = q->head;
    while (current->next && current->next->data.processPriority <= process.processPriority) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

PCB dequeue(PriorityQueue* q) {
    PCB empty = {0};
    if (!q || !q->head) {
        fprintf(stderr, "Queue underflow: No process to dequeue\n");
        return empty;
    }

    Node* temp = q->head;
    PCB process = temp->data;
    q->head = temp->next;
    free(temp);
    return process;
}

PCB peek(PriorityQueue* q) {
    if (!q || !q->head) {
        return (PCB){0};
    }
    return q->head->data;
}

int isEmpty(PriorityQueue* q) {
    return !q || q->head == NULL;
}

void destroyQueue(PriorityQueue* q) {
    Node* temp;
    while (q && q->head) {
        temp = q->head;
        q->head = q->head->next;
        free(temp);
    }
    if (q) free(q);
}

#endif
