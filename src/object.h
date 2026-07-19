#ifndef OBJECT_H
#define OBJECT_H

#include "base.h"
#include "shapes.h"
#include "texture.h"
#include <stdbool.h>

typedef struct {
    bool collide;
    double t;
    LightIntensity intensity;
} CollideColorResult;

typedef struct _Object {
    Shape *shape;
    Texture texture;
    
} Object;

CollideColorResult collideObject(const struct _Object *self, const ReversedRay *ray);

Object newObject(Shape *shape, Texture texture);
#endif