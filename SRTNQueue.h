#ifndef SRTN_QUEUE_H
#define SRTN_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "PriorityQueue.h"



typedef struct Node {
    PCB data;
    struct Node* next;
} Node;

typedef struct PriorityQueueSRTN {
    Node* head;
} PriorityQueueSRTN;

PriorityQueueSRTN* createQueue() {
    PriorityQueueSRTN* q = (PriorityQueueSRTN*)malloc(sizeof(PriorityQueueSRTN));
    if (!q) {
        perror("Failed to allocate memory for queue");
        return NULL;
    }
    q->head = NULL;
    return q;
}

void enqueue(PriorityQueueSRTN* q, PCB process) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Failed to allocate memory for node");
        return;
    }
    newNode->data = process;
    newNode->next = NULL;

    if (!q->head || process.remainingTime < q->head->data.remainingTime) {
        newNode->next = q->head;
        q->head = newNode;
        return;
    }

    Node* current = q->head;
    while (current->next && current->next->data.remainingTime <= process.remainingTime) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

PCB dequeue(PriorityQueueSRTN* q) {
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

PCB peek(PriorityQueueSRTN* q) {
    if (!q || !q->head) {
        return (PCB){0};
    }
    return q->head->data;
}

int isEmpty(PriorityQueueSRTN* q) {
    return !q || q->head == NULL;
}

void destroyQueue(PriorityQueueSRTN* q) {
    Node* temp;
    while (q && q->head) {
        temp = q->head;
        q->head = q->head->next;
        free(temp);
    }
    if (q) free(q);
}

#endif
