typedef struct QueueNodeNormal {
    PCB pcb;
    struct QueueNodeNormal* next;
} QueueNodeNormal;
typedef struct PCBQueueNormal {
    QueueNodeNormal* front;
    QueueNodeNormal* rear;
    int queue_size;
} PCBQueueNormal;

void initQueueN(PCBQueueNormal* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->queue_size = 0;
}

void enqueueN(PCBQueueNormal* queue, PCB newPCB) {
    QueueNodeNormal* newNode = (QueueNodeNormal*)malloc(sizeof(QueueNodeNormal));
    newNode->pcb = newPCB;    
    newNode->next = NULL;      

    if (queue->rear == NULL) { 
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;  
        queue->rear = newNode;        
    }

    queue->queue_size++;  
}

PCB dequeueN(PCBQueueNormal* queue) {
    if (queue->queue_size == 0) {  
        printf("Queue is empty! Cannot dequeue process.\n");
        PCB emptyPCB = {0}; 
        return emptyPCB;
    }

    QueueNodeNormal* temp = queue->front;  
    PCB dequeuedPCB = temp->pcb;     
    queue->front = queue->front->next;  

    if (queue->front == NULL) { 
        queue->rear = NULL;
    }

    free(temp);

    queue->queue_size--;
    return dequeuedPCB;
}

PCB peekN(PCBQueueNormal* queue) {
    if (queue->queue_size == 0) {
        printf("Queue is empty! Cannot peek process.\n");
        PCB emptyPCB = {0};
        return emptyPCB;
    }
    
    return queue->front->pcb;
}