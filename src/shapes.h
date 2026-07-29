#ifndef SHAPES_H
#define SHAPES_H

#include "base.h"

typedef struct {
    bool collide;
    double t;
    Vector normalVector;
} CollideResult;

typedef struct _Shape {
    CollideResult (*detectCollide)(struct _Shape *, ReversedRay *);
    void (*destroySelf) (struct _Shape *);
} Shape;

typedef struct {
    Shape shape;
    Point p1, p2, p3;
} Triangle;

Shape *newTriangle(Point p1, Point p2, Point p3);

typedef struct {
    Shape shape;
    Point center;
    double radius;
} Sphere;

Shape *newSphere(Point center, double radius);

typedef struct {
    Triangle tri;
    Vector normalVector;
} DirectedTriangle;

Shape *newDirectedTriangle(Point p1, Point p2, Point p3, Vector normalVector);

typedef struct {
    Shape shape;
    Point *vertexes;
} ConvexPolygon;

Shape *newConvexPolygon(Point *vertexes);

typedef struct {
    Shape shape;
    Shape **elements;
} Group;

Shape *newGroup(Shape *elements);

#endif