#ifndef SHAPES_H
#define SHAPES_H

#include "base.h"

typedef struct {
    bool collide;
    double t;
} CollisionResult;

typedef struct _Shape {
    CollisionResult (*detectCollide)(const struct _Shape *, const ReversedRay *);
    Vector (*reflectVector)(const struct _Shape *, Point, const ReversedRay *);
} Shape;
/*
 * For the fact that line segment has no "normal vector" for a given point, unfortunately these has
 * to be deprecated.
 */

/*
typedef struct {
    Shape shape;
    Point start, end;
    double width;
} LineSegment;

LineSegment newLineSegment(Point start, Point end, double width);

typedef struct {
    Shape shape;
    Point *points;
    Point center;
    double width;
} Cube;

Cube *newCube(double length, Point center, double width);
void freeCube(Cube *cube);

void rotateCubeX(Cube *cube, double angle);
void rotateCubeY(Cube *cube, double angle);
void rotateCubeZ(Cube *cube, double angle);

void moveCube(Cube *cube, Vector displacement);
void setCubePosition(Cube *cube, Point pos);
*/
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

#endif