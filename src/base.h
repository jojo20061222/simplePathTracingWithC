#ifndef BASE_H
#define BASE_H

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

Vector cross(Vector a, Vector b);
Vector plus(Vector a, Vector b);
Vector minus(Vector a, Vector b);
Vector scalerMultiplication(double c, Vector a);
Vector unitVector(Vector vec);
double dot(Vector a, Vector b);

typedef struct {
    Point start;
    Vector direction;
    double powerFactor;
} ReversedRay;

#endif