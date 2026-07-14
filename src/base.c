#include "base.h"
#include <math.h>

static const double error = 1e-8;

Vector unitVector(Vector vec) {
    double length = sqrt(dot(vec, vec));
    if (length < error) return (Vector){0, 0, 0};
    return (Vector){vec.x / length, vec.y / length, vec.z / length};
}