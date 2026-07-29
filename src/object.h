#ifndef OBJECT_H
#define OBJECT_H

#include "base.h"
#include "shapes.h"
#include "textures.h"
#include <stdbool.h>

typedef struct Object Object;

Object *newObject(Shape *shape, Texture texture);

CollideResult collideObject(Object *, ReversedRay *);

Complex getRefractiveIndexInside(Object *self, double lambda);

Complex getRefractiveIndexOutside(Object *self, double lambda);

double getRoughness(Object *self);

bool getMetalic(Object *self);

bool getTransparent(Object *self);

double getBaseColor(Object *self, double lambda);

#endif