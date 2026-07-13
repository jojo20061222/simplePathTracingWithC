#include "render.h"
#include "dynamicArray.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Color *accessScreen(VirtualScreen *screen, int x, int y) {
    if (x < 0 || x >= screen->width) {
        printf("invalid access of x: %d\n", x);
        return NULL;
    }
    if (y < 0 || y >= screen->height) {
        printf("invalid access of y: %d\n", y);
    }
    return &screen->buffer[y * screen->width + x];
}

VirtualScreen *newScreen(int width, int height) {
    VirtualScreen *result = malloc(sizeof(VirtualScreen));
    result->width = width;
    result->height = height;
    result->buffer = calloc(width * height, sizeof(Color *));
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
    ReversedRay **array;
} RayQueue;

RayQueue newQueue() {
    const int initCapacity = 1024;
    RayQueue result;
    result.capacity = initCapacity;
    result.head = 0;
    result.rear = 0;
    result.size = 0;
    result.array = malloc(sizeof(Object *) * initCapacity);
    return result;
}
void freeQueue(RayQueue queue) { free(queue.array); }

void push(RayQueue *queue, ReversedRay *ray) {
    queue->array[queue->rear++] = ray;
    queue->size++;
    if (queue->size == queue->capacity) {
        ReversedRay **moveTo = malloc(sizeof(Object *) * queue->capacity * 2);
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

ReversedRay *pop(RayQueue *queue) {
    assert(queue->size > 0);
    queue->size--;
    ReversedRay *result = queue->array[queue->head++];
    if (queue->head >= queue->capacity) queue->head = 0;
    return result;
}

int size(RayQueue *queue) { return queue->size; }

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
    for (int i = 0; i < objectsCount; i++) {
        CollideColorResult collisionResult = collideObject(objects[i], initRay);
        if (!collisionResult.collide) continue;
        if (collisionResult.t < minT) {
            minT = collisionResult.t;
            *accessScreen(s, x, y) = collisionResult.color;
        }
    }
}

void renderScreen(Point *camera, VirtualScreen *s, const Object **objects) {
    int objectsCount = getSize(objects);
    for (int i = 0; i < s->width; i++) {
        for (int j = 0; j < s->height; j++) {
            *accessScreen(s, i, j) = (Color)0U;
            renderPixel(camera, s, i, j, objects, objectsCount);
        }
    }
}