#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
// #include <helpers.h>

typedef struct QueueNode
{
    PCB data;
    struct QueueNode *next;
} QueueNode;

typedef struct CircularQueue
{
    QueueNode *tail;    // tail->next is the head
    QueueNode *current; // pointer to "currently running" process
    int size;
} CircularQueue;

void initQueue(CircularQueue *q)
{
    q->tail = NULL;
    q->current = NULL;
    q->size = 0;
}

void enqueueCirc(CircularQueue *q, PCB data)
{
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    newNode->data = data;
    newNode->next = NULL;

    if (q->tail == NULL)
    {
        newNode->next = newNode; // if empty
        q->tail = newNode;
        q->current = newNode;
    }
    else
    {
        newNode->next = q->tail->next; // link to head
        q->tail->next = newNode;
        q->tail = newNode;
    }

    q->size++;
}

void rotate(CircularQueue *q)
{
    if (q->current != NULL)
        q->current = q->current->next;
}

void removeCurrent(CircularQueue *q)
{
    if (q->size == 0 || q->current == NULL)
        return;

    QueueNode *prev = q->current;
    while (prev->next != q->current)
    {
        prev = prev->next;
    }

    if (q->current == q->current->next)
    {
        free(q->current);
        q->current = q->tail = NULL;
    }
    else
    {
        prev->next = q->current->next;

        if (q->current == q->tail)
            q->tail = prev;

        QueueNode *toDelete = q->current;
        q->current = q->current->next;
        free(toDelete);
    }

    q->size--;
}

PCB *peekCurrent(CircularQueue *q)
{
    return (q->current != NULL) ? &q->current->data : NULL;
}

void printQueue(CircularQueue *q)
{
    if (q->size == 0)
    {
        printf("Queue is empty.\n");
        return;
    }

    QueueNode *temp = q->tail->next;
    printf("Queue (current = PID %d):\n", q->current->data.processID);
    int i = 0;
    do
    {
        printf("  [%d] PID: %d | Remaining: %d\n", i++, temp->data.processID, temp->data.remainingTime);
        temp = temp->next;
    } while (temp != q->tail->next);
}