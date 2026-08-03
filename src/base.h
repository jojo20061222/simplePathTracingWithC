#ifndef BASE_H
#define BASE_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

static inline Vector vecCross(Vector a, Vector b) {
    return (Vector){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static inline Vector vecAdd(Vector a, Vector b) {
    return (Vector){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vector vecMinus(Vector a, Vector b) {
    return (Vector){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vector vecNegative(Vector v) { return (Vector){-v.x, -v.y, -v.z}; }

static inline Vector vecMultiply(double c, Vector a) { return (Vector){c * a.x, c * a.y, c * a.z}; }

static inline double vecDot(Vector a, Vector b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static const double error = 1e-8;

static inline Vector unitVector(Vector vec) {
    double length = sqrt(vecDot(vec, vec));
    if (length < error) return (Vector){0, 0, 0};
    return (Vector){vec.x / length, vec.y / length, vec.z / length};
}

static inline double lengthOf(Vector v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

typedef struct {
    Point start;
    Vector direction;
    double waveLength;
} ReversedRay;

static inline ReversedRay newRay(Point start, Vector direction, double waveLength) {
    return (ReversedRay){start, unitVector(direction), waveLength};
}

typedef double LightIntensity;

typedef struct {
    double R, I;
} Complex;

static inline Complex cplxAdd(Complex a, Complex b) { return (Complex){a.R + b.R, a.I + b.I}; }

static inline Complex cplxMinus(Complex a, Complex b) { return (Complex){a.R - b.R, a.I - b.I}; }

static inline Complex cplxMultiply(Complex a, Complex b) {
    // (a + bi)(c + di) = (ac - bd) + (ad + bc)i
    return (Complex){a.R * b.R - a.I * b.I, a.R * b.I + a.I * b.R};
}

static inline Complex cplxMultiplyByReal(double r, Complex z) {
    return (Complex){r * z.R, r * z.I};
}

static inline Complex cplxDivide(Complex a, Complex b) {
    // (a + bi) / (c + di) = (a + bi)(c - di) / (c^2 + d^2)
    // = (ac + bd) / (c^2 + d^2) + (bc - ad)i / (c^2 + d^2)
    double deno = b.R * b.R + b.I * b.I;
    return (Complex){(a.R * b.R + a.I * b.I) / deno, (a.I * b.R - a.R * b.I) / deno};
}

static inline double cplxModulus(Complex z) { return sqrt(z.R * z.R + z.I * z.I); }

static inline double cplxArg(Complex z) {
    if (z.R == 0 && z.I == 0)
        return NAN; // x = 0 and y = 0
    else if (z.R == 0) {
        if (z.I > 0)
            return M_PI / 2; // x = 0 and y > 0
        else
            return -M_PI / 2; // x = 0 and y < 0
    }
    else if (z.R > 0) { // x > 0
        return atan(z.I / z.R);
    }
    else if (z.I >= 0) { // x < 0 and y >= 0
        return atan(z.I / z.R) + M_PI;
    }
    return atan(z.I / z.R) - M_PI; // x < 0 and y < 0
}

static inline double lerp(double x1, double x2, double x, double y1, double y2) {
    // since (y - y1) / (x - x1) = (y2 - y) / (x2 - x)
    // we have (y - y1)(x2 - x) = (y2 - y)(x - x1)
    // x2y - xy - x2y1 + y1x = y2x - x1y2 - xy + x1y
    // (x2 - x1)y = x2y1 - x1y2 + y2x - y1x = y1(x2 - x) + y2(x - x1)
    if (fabs(x2 - x1) < error) return y1;
    return (y1 * (x2 - x) + y2 * (x - x1)) / (x2 - x1);
}

#endif