#ifndef OBJECT_H
#define OBJECT_H

#include "base.h"
#include "shapes.h"
#include <stdbool.h>

typedef struct {
    bool collide;
    double t;
    Color color;
} CollideColorResult;

typedef struct _Object {
    Shape *shape;
    Color color;
    double diffuseFactor, specularFactor;
} Object;

CollideColorResult collideObject(const struct _Object *self, const ReversedRay *ray);

Object newObject(Shape *shape, Color color, double diffuseFactor, double specularFactor);

#endif