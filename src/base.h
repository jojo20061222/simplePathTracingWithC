#ifndef BASE_H
#define BASE_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

typedef union {
    struct {
        uint8_t R, G, B, A;
    } color;
    uint32_t pixel;
} ScreenColor;

typedef struct {
    double x, y, z;
} Tuple;

typedef Tuple Vector;
typedef Tuple Point;

static inline Vector cross(Vector a, Vector b) {
    return (Vector){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static inline Vector add(Vector a, Vector b) { return (Vector){a.x + b.x, a.y + b.y, a.z + b.z}; }

static inline Vector minus(Vector a, Vector b) { return (Vector){a.x - b.x, a.y - b.y, a.z - b.z}; }

static inline Vector multiply(double c, Vector a) { return (Vector){c * a.x, c * a.y, c * a.z}; }

static inline double dot(Vector a, Vector b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static const double error = 1e-8;

static inline Vector unitVector(Vector vec) {
    double length = sqrt(dot(vec, vec));
    if (length < error) return (Vector){0, 0, 0};
    return (Vector){vec.x / length, vec.y / length, vec.z / length};
}

static inline double lengthOf(Vector v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

typedef struct {
    Point start;
    Vector direction;
    double waveLength;
    double refractiveIndex;
} ReversedRay;

ReversedRay newRay(Point start, Vector direction, double waveLength, double refractiveIndex);

typedef struct {
    int capacity;
    int size;
    int head, rear;
    void **array;
} Queue;

Queue newQueue();

void freeQueue(Queue);

void push(Queue *, void *);

void *pop(Queue *);

int size(Queue *);

typedef double LightIntensity;

#endif