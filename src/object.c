#include "object.h"
#include <stdlib.h>

/*
 * @author seaofvoices
 * @brief This file defines attributes an object should possess and provide interfaces to others.
 */

typedef struct Object {
    Shape *shape;
    Interface *interface;
    bool swapped;
} Object;

Object *newObject(Shape *shape, Interface *interface) {
    Object *obj = malloc(sizeof(*obj));
    if (obj == NULL) {
        fprintf(stderr, "malloc obj failed\n");
        exit(1);
    }
    obj->shape = shape;
    obj->interface = interface;
    obj->swapped = false;
    return obj;
}

void destroyObject(Object *obj) { free(obj); }

CollideResult collideObject(Object *obj, ReversedRay *ray) {
    return detectCollide(obj->shape, ray);
}

Real getObjectInsideSigmaS(Object *self, Real lambda) {
    if (self->swapped) return getInterfaceOutsideSigmaS(self->interface, lambda);
    return getInterfaceInsideSigmaS(self->interface, lambda);
}

Vector sampleObjectInsideNextDirection(Object *self, Vector currentDirection, MT19937 *mt) {
    if (self->swapped)
        return sampleInterfaceOutsideNextDirection(self->interface, currentDirection, mt);
    return sampleInterfaceInsideNextDirection(self->interface, currentDirection, mt);
}

Complex getObjectInsideRefractiveIndex(Object *self, Real lambda) {
    if (self->swapped) return getInterfaceOutsideRefractiveIndex(self->interface, lambda);
    return getInterfaceInsideRefractiveIndex(self->interface, lambda);
}

Complex getObjectOutsideRefractiveIndex(Object *self, Real lambda) {
    if (self->swapped) return getInterfaceInsideRefractiveIndex(self->interface, lambda);
    return getInterfaceOutsideRefractiveIndex(self->interface, lambda);
}

Real getObjectRoughness(Object *self) { return getInterfaceRoughness(self->interface); }

bool isObjectInsideMetallic(Object *self) {
    if (self->swapped) return isInterfaceOutsideMetallic(self->interface);
    return isInterfaceInsideMetallic(self->interface);
}

bool isObjectInsideFullyTransparent(Object *self) {
    if (self->swapped) return isInterfaceOutsideFullyTransparent(self->interface);
    return isInterfaceInsideFullyTransparent(self->interface);
}

bool isObjectInsideTransparent(Object *self) {
    if (self->swapped) return isInterfaceOutsideTransparent(self->interface);
    return isInterfaceInsideTransparent(self->interface);
}

bool isObjectInsideHalfTransparent(Object *self) {
    if (self->swapped) return isInterfaceOutsideHalfTransparent(self->interface);
    return isInterfaceInsideHalfTransparent(self->interface);
}

bool isObjectOutsideHalfTransparent(Object *self) {
    if (self->swapped) return isInterfaceInsideHalfTransparent(self->interface);
    return isInterfaceOutsideHalfTransparent(self->interface);
}

Real getObjectInsideBaseColor(Object *self, Real lambda) {
    if (self->swapped) return getInterfaceOutsideBaseColor(self->interface, lambda);
    return getInterfaceInsideBaseColor(self->interface, lambda);
}

Vector getNormalVector(Object *self, Point p) {
    Vector N = NormalVectorOfPoint(self->shape, p);
    if (self->swapped) return vecNegative(N);
    return N;
}

void swapInterface(Object *self) {
    self->swapped = !self->swapped;
}
