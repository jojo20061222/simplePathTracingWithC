#ifndef BASE_H
#define BASE_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef DOUBLE_AS_REAL
#define REAL_FMT "%lf"
typedef double Real;
#else
#define REAL_FMT "%f"
typedef float Real;
#endif

static inline Real rabs(Real x) {
#ifdef DOUBLE_AS_REAL
    return fabs(x);
#else
    return fabsf(x);
#endif
}

static inline Real rexp(Real x) {
#ifdef DOUBLE_AS_REAL
    return exp(x);
#else
    return expf(x);
#endif
}

static inline Real rsin(Real x) {
#ifdef DOUBLE_AS_REAL
    return sin(x);
#else
    return sinf(x);
#endif
}

static inline Real rcos(Real x) {
#ifdef DOUBLE_AS_REAL
    return cos(x);
#else
    return cosf(x);
#endif
}

static inline Real rmin(Real a, Real b) {
#ifdef DOUBLE_AS_REAL
    return fmin(a, b);
#else
    return fminf(a, b);
#endif
}

static inline Real rmax(Real a, Real b) {
#ifdef DOUBLE_AS_REAL
    return fmax(a, b);
#else
    return fmaxf(a, b);
#endif
}

static inline Real rlog(Real x) {
#ifdef DOUBLE_AS_REAL
    return log(x);
#else
    return logf(x);
#endif
}

typedef union {
    struct {
        uint8_t R, G, B, A;
    } color;
    uint32_t pixel;
} ScreenColor;

typedef struct {
    Real x, y, z;
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

// return -v
static inline Vector vecNegative(Vector v) { return (Vector){-v.x, -v.y, -v.z}; }

// scaler multiplication of vectors
static inline Vector vecMultiply(Real c, Vector a) { return (Vector){c * a.x, c * a.y, c * a.z}; }

static inline Real vecDot(Vector a, Vector b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static const Real error = 1e-6;

static inline Real lengthOf(Vector v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

static inline Real length2Of(Vector v) { return v.x * v.x + v.y + v.y + v.z * v.z; }

// normalize the vector
static inline Vector unitVector(Vector vec) {
    Real length = lengthOf(vec);
    if (length < error) return (Vector){0, 0, 0};
    return (Vector){vec.x / length, vec.y / length, vec.z / length};
}

typedef struct {
    Point start;
    Vector direction;
    Real wavelength;
} ReversedRay;

static inline ReversedRay getRay(Point start, Vector direction, Real wavelength) {
    return (ReversedRay){start, unitVector(direction), wavelength};
}

typedef Real LightIntensity;

typedef struct {
    Real R, I;
} Complex;

static inline Complex cplxAdd(Complex a, Complex b) { return (Complex){a.R + b.R, a.I + b.I}; }

static inline Complex cplxMinus(Complex a, Complex b) { return (Complex){a.R - b.R, a.I - b.I}; }

static inline Complex cplxMultiply(Complex a, Complex b) {
    // (a + bi)(c + di) = (ac - bd) + (ad + bc)i
    return (Complex){a.R * b.R - a.I * b.I, a.R * b.I + a.I * b.R};
}

static inline Complex cplxMultiplyByReal(Real r, Complex z) { return (Complex){r * z.R, r * z.I}; }

static inline Complex cplxDivide(Complex a, Complex b) {
    // (a + bi) / (c + di) = (a + bi)(c - di) / (c^2 + d^2)
    // = (ac + bd) / (c^2 + d^2) + (bc - ad)i / (c^2 + d^2)
    Real deno = b.R * b.R + b.I * b.I;
    return (Complex){(a.R * b.R + a.I * b.I) / deno, (a.I * b.R - a.R * b.I) / deno};
}

static inline Real cplxModulus(Complex z) { return sqrt(z.R * z.R + z.I * z.I); }

static inline Real cplxArg(Complex z) {
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

static inline Real lerp(Real a, Real b, Real t) { return a + t * (b - a); }

// linearly map x from [x1, x2] to [y1, y2]
static inline Real lerpInterval(Real x1, Real x2, Real x, Real y1, Real y2) {
    // since (y - y1) / (x - x1) = (y2 - y) / (x2 - x)
    // we have (y - y1)(x2 - x) = (y2 - y)(x - x1)
    // x2y - xy - x2y1 + y1x = y2x - x1y2 - xy + x1y
    // (x2 - x1)y = x2y1 - x1y2 + y2x - y1x = y1(x2 - x) + y2(x - x1)
    if (rabs(x2 - x1) < error) return y1;
    return (y1 * (x2 - x) + y2 * (x - x1)) / (x2 - x1);
}

static inline Vector toWorld(Vector local, Vector N) {
    Vector helper = (rabs(N.x) > 0.9) ? ((Vector){0, 1, 0}) : ((Vector){1, 0, 0});
    Vector T = unitVector(vecCross(helper, N));
    Vector B = vecCross(N, T);
    // project the unturned to turned
    Vector nxtDirection = (Vector){local.x * T.x + local.y * B.x + local.z * N.x,
                                   local.x * T.y + local.y * B.y + local.z * N.y,
                                   local.x * T.z + local.y * B.z + local.z * N.z};
    return nxtDirection;
}

#endif