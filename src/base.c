#include "base.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>

static const double error = 1e-8;

Vector unitVector(Vector vec) {
    double length = sqrt(dot(vec, vec));
    if (length < error) return (Vector){0, 0, 0};
    return (Vector){vec.x / length, vec.y / length, vec.z / length};
}

Queue newQueue() {
    const int initCapacity = 1024;
    Queue result;
    result.capacity = initCapacity;
    result.head = 0;
    result.rear = 0;
    result.size = 0;
    result.array = malloc(sizeof(void *) * initCapacity);
    return result;
}

void freeQueue(Queue queue) { free(queue.array); }

void push(Queue *queue, void *newobj) {
    queue->array[queue->rear++] = newobj;
    queue->size++;
    if (queue->size == queue->capacity) {
        void **moveTo = malloc(sizeof(void *) * queue->capacity * 2);
        for (int i = 0; i < queue->size; i++) {
            moveTo[i] = queue->array[(queue->head + i) % queue->capacity];
        }
        free(queue->array);
        queue->capacity *= 2;
        queue->array = moveTo;
        queue->head = 0;
        queue->rear = queue->size;
    }
    else if (queue->rear == queue->capacity) {
        queue->rear = 0;
    }
}

void *pop(Queue *queue) {
    assert(queue->size > 0);
    queue->size--;
    ReversedRay *result = queue->array[queue->head++];
    if (queue->head >= queue->capacity) queue->head = 0;
    return result;
}

int size(Queue *queue) { return queue->size; }