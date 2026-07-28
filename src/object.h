#ifndef OBJECT_H
#define OBJECT_H

#include "base.h"
#include "shapes.h"
#include "textures.h"
#include <stdbool.h>

typedef struct _Object {
    Shape *shape;
    Texture texture;
} Object;

Object newObject(Shape *shape, Texture texture);

CollideResult collideObject(Object *, ReversedRay *);

#endif