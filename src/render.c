#include "render.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

VirtualScreen *newScreen(int width, int height) {
    VirtualScreen *result = malloc(sizeof(VirtualScreen));
    result->width = width;
    result->height = height;
    result->buffer = calloc(width * height, sizeof(ScreenColor *));
    return result;
}

void freeScreen(VirtualScreen *screen) {
    free(screen->buffer);
    free(screen);
    return;
}

typedef struct {
    int capacity;
    int size;
    int head, rear;
    void **array;
} Queue;

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

void renderPixel(Point *camera, VirtualScreen *s, int x, int y, const Object **objects,
                 int objectsCount) {
    /*
     * Some basic idea about this function is to generate a reversed ray start from the camera and
     * goes through a pixel, then see if it collide with an object. If it collides with an object
     * then do something consequently.
     */
    ReversedRay *initRay = malloc(sizeof(ReversedRay));
    *initRay = (ReversedRay){*camera, minus((Vector){x, y, 0}, *camera), 1.0};
    double minT = INFINITY;
    CollideColorResult finalCollisionResult;
    for (int i = 0; i < objectsCount; i++) {
        CollideColorResult collisionResult = collideObject(objects[i], initRay);
        if (!collisionResult.collide) continue;
        if (collisionResult.t < minT) {
            minT = collisionResult.t;
            finalCollisionResult = collisionResult;
        }
    }
    // TODO: to modify the program to calculate the reflection
    ScreenColor finalColor = {(uint8_t)(finalCollisionResult.color.R * 255),
                              (uint8_t)(finalCollisionResult.color.G * 255),
                              (uint8_t)(finalCollisionResult.color.B * 255)};
    *accessScreen(s, x, y) = finalColor;
}

void renderScreen(Point *camera, VirtualScreen *s, const Object **objects, int objectsCount) {
    for (int i = 0; i < s->width; i++) {
        for (int j = 0; j < s->height; j++) {
            *accessScreen(s, i, j) = (ScreenColor)0U;
            renderPixel(camera, s, i, j, objects, objectsCount);
        }
    }
}