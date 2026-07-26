#ifndef SHAPES_H
#define SHAPES_H

#include "base.h"

typedef struct {
    bool collide;
    double t;
} CollideResult;

typedef struct _Shape {
    CollideResult (*detectCollide)(const struct _Shape *, const ReversedRay *);
    Vector (*normalVector)(const struct _Shape *, Point);
} Shape;

typedef struct {
    Shape shape;
    Point p1, p2, p3;
} Triangle;

Triangle newTriangle(Point p1, Point p2, Point p3);

typedef struct {
    Shape shape;
    Point center;
    double radius;
} Sphere;

Sphere newSphere(Point center, double radius);

typedef struct {
    Triangle tri;
    Vector normalVector;
} DirectedTriangle;

DirectedTriangle newDirectedTriangle(Point p1, Point p2, Point p3, Vector normalVector);

typedef struct {
    Shape shape;
    Point vertexes;
    Point center;
    double sizeFactor;
} Diamond;

#endif