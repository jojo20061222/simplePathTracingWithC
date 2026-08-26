#ifndef SHAPES_H
#define SHAPES_H

#include "base.h"
#include "random.h"

typedef struct {
    bool collide;        // if the ray collide the shape
    Real distance;       // the distance between the start of the ray and the collide point
    Vector normalVector; // the normal vector of the surface on the collide point
} CollideResult;

typedef struct Shape Shape;

typedef struct Triangle Triangle;

typedef struct Sphere Sphere;

typedef struct ConvexPolygon ConvexPolygon;

typedef struct Polygon Polygon;

typedef struct Group Group;

Shape *newTriangle(Point p1, Point p2, Point p3);

Shape *newSphere(Point center, Real radius);

Shape *newConvexPolygon(Point *vertexes);

Shape *newPolygon(Point *vertexes);

Shape *newGroup(Shape **elements);

CollideResult detectCollide(Shape *shape, ReversedRay *ray);

void destroyShape(Shape *shape);

Point randomPoint(Shape *shape, MT19937 *mt);

Real PDFofPoint(Shape *self, Point point);

Vector NormalVectorOfPoint(Shape *self, Point point);

#endif