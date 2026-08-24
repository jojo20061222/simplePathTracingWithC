#ifndef OBJECT_H
#define OBJECT_H

#include "base.h"
#include "shapes.h"
#include "textures.h"
#include <stdbool.h>

typedef struct Object Object;

Object *newObject(Shape *shape, Texture *texture);

void destroyObject(Object *obj);

CollideResult collideObject(Object *, ReversedRay *);

Real getObjectSigmaS(Object *self, Real lambda);

Vector sampleObjectNextDirection(Object *self, Vector currentDirection);

Complex getRefractiveIndexInside(Object *self, Real lambda);

Complex getRefractiveIndexOutside(Object *self, Real lambda);

Real getRoughness(Object *self);

bool isObjectMetallic(Object *self);

bool isObjectFullyTransparent(Object *self);

bool isObjectHalfTransparent(Object *self);

Real getBaseColor(Object *self, Real lambda);

Vector getNormalVector(Object *self, Point p);


#endif