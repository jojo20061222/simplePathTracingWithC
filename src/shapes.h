#ifndef SHAPES_H
#define SHAPES_H

#include "base.h"

typedef struct {
    bool collide;
    Real distance;
    Vector normalVector;
} CollideResult;

typedef struct Shape Shape;

typedef struct Triangle Triangle;

typedef struct Sphere Sphere;

typedef struct DirectedTriangle DirectedTriangle;

typedef struct ConvexPolygon ConvexPolygon;

typedef struct Group Group;

Shape *newTriangle(Point p1, Point p2, Point p3);

Shape *newSphere(Point center, Real radius);

Shape *newDirectedTriangle(Point p1, Point p2, Point p3, Vector normalVector);

Shape *newConvexPolygon(Point *vertexes);

Shape *newGroup(Shape **elements);

CollideResult detectCollide(Shape *shape, ReversedRay *ray);

void destroyShape(Shape *shape);

#endif