#include <stdlib.h>
#include "queue.h"

QUEUE* newqueue(unsigned int size) {
    QUEUE* queue = (QUEUE*) malloc(sizeof(struct S_QUEUE));

    queue->maxElements = size;
    queue->front       = queue->size = 0;
    queue->rear        = size - 1;
    queue->data        = (void*) malloc(queue->maxElements * sizeof(void*));

    return queue;
}

void freequeue(QUEUE* queue) {
    if (queue) {
        if (queue->data) {
            free(queue->data);
        }

        free(queue);
    }
}

int isFull(QUEUE* queue) {
    return (queue->size == queue->maxElements);
}

int isEmpty(QUEUE* queue) {
    return (queue->size == 0);
}

unsigned int enqueue(QUEUE* queue, void *element) {
    if (isFull(queue))
        return -1;

    queue->rear = (queue->rear + 1) % queue->maxElements;
    queue->data[queue->rear] = element;
    queue->size = queue->size + 1;

    return queue->rear;
}

void* dequeue(QUEUE* queue) {
    if (isEmpty(queue))
        return NULL;

    void* element = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->maxElements;
    queue->size = queue->size - 1;

    return element;
}

void* front(QUEUE* queue) {
    if (isEmpty(queue))
        return NULL;

    return queue->data[queue->front];
}

void* rear(QUEUE* queue) {
    if (isEmpty(queue))
        return NULL;

    return queue->data[queue->rear];
}