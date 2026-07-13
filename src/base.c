#include "base.h"
#include <math.h>

static const double error = 1e-8;

Vector cross(Vector a, Vector b) {
    return (Vector){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vector plus(Vector a, Vector b) { return (Vector){a.x + b.x, a.y + b.y, a.z + b.z}; }

Vector minus(Vector a, Vector b) { return (Vector){a.x - b.x, a.y - b.y, a.z - b.z}; }

Vector scalerMultiplication(double c, Vector a) { return (Vector){c * a.x, c * a.y, c * a.z}; }

double dot(Vector a, Vector b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vector unitVector(Vector vec) {
    double length = sqrt(dot(vec, vec));
    if (length < error) return (Vector){0, 0, 0};
    return (Vector){vec.x / length, vec.y / length, vec.z / length};
}