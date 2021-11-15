#ifndef CREXX_QUEUE_H
#define CREXX_QUEUE_H

typedef struct S_QUEUE {
    unsigned int front, rear, size;
    unsigned int maxElements;
    void         **data;
} QUEUE;

QUEUE* newqueue(unsigned int size);
void   freequeue(QUEUE* queue);

int isFull(QUEUE* queue);
int isEmpty(QUEUE* queue);

unsigned int enqueue(QUEUE* queue, void* element);
void* dequeue(QUEUE* queue);
void* front(QUEUE* queue);
void* rear(QUEUE* queue);

#endif //CREXX_QUEUE_H
