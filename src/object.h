#ifndef OBJECT_H
#define OBJECT_H

#include "base.h"
#include "shapes.h"
#include <stdbool.h>

typedef struct {
    double R, G, B;
} ObjectColor;

typedef struct {
    bool collide;
    double t;
    ObjectColor color;
} CollideColorResult;

typedef struct _Object {
    Shape *shape;
    ObjectColor color;
    double diffuseFactor;
} Object;

CollideColorResult collideObject(const struct _Object *self, const ReversedRay *ray);

Object newObject(Shape *shape, ObjectColor color, double diffuseFactor);

#endif