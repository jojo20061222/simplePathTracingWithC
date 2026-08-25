#ifndef OBJECT_H
#define OBJECT_H

#include "base.h"
#include "shapes.h"
#include "textures.h"
#include <stdbool.h>

typedef struct Object Object;

Object *newObject(Shape *shape, Interface *interface);

void destroyObject(Object *obj);

CollideResult collideObject(Object *, ReversedRay *);

Real getObjectInsideSigmaS(Object *self, Real lambda);

Vector sampleObjectInsideNextDirection(Object *self, Vector currentDirection, MT19937 *mt);

Complex getObjectInsideRefractiveIndex(Object *self, Real lambda);

Complex getObjectOutsideRefractiveIndex(Object *self, Real lambda);

Real getObjectRoughness(Object *self);

bool isObjectInsideMetallic(Object *self);

bool isObjectInsideFullyTransparent(Object *self);

bool isObjectInsideHalfTransparent(Object *self);

Real getObjectInsideBaseColor(Object *self, Real lambda);

Vector getNormalVector(Object *self, Point p);

#endif