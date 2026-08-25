#include "object.h"
#include <stdlib.h>

/*
 * @author seaofvoices
 * @brief This file defines attributes an object should possess and provide interfaces to others.
 */

typedef struct Object {
    Shape *shape;
    Interface *interface;
} Object;

Object *newObject(Shape *shape, Interface *interface) {
    Object *obj = malloc(sizeof(*obj));
    if (obj == NULL) {
        fprintf(stderr, "malloc obj failed\n");
        exit(1);
    }
    obj->shape = shape;
    obj->interface = interface;
    return obj;
}

void destroyObject(Object *obj) { free(obj); }

CollideResult collideObject(Object *obj, ReversedRay *ray) {
    return detectCollide(obj->shape, ray);
}

Real getObjectSigmaS(Object *self, Real lambda) {
    return getInterfaceInsideSigmaS(self->interface, lambda);
}

Vector sampleObjectNextDirection(Object *self, Vector currentDirection, MT19937 *mt) {
    return sampleInterfaceInsideNextDirection(self->interface, currentDirection, mt);
}

Complex getRefractiveIndex(Object *self, Real lambda) {
    return getInterfaceInsideRefractiveIndex(self->interface, lambda);
}

Complex getRefractiveIndexInside(Object *self, Real lambda) {
    return getInterfaceInsideRefractiveIndex(self->interface, lambda);
}

Real getRoughness(Object *self) { return getInterfaceRoughness(self->interface); }

bool isObjectMetallic(Object *self) { return isInterfaceInsideMetallic(self->interface); }

bool isObjectFullyTransparent(Object *self) {
    return isInterfaceInsideFullyTransparent(self->interface);
}

bool isObjectHalfTransparent(Object *self) {
    return isInterfaceInsideHalfTransparent(self->interface);
}

Real getBaseColor(Object *self, Real lambda) {
    return getInterfaceInsideBaseColor(self->interface, lambda);
}

Vector getNormalVector(Object *self, Point p) { return NormalVectorOfPoint(self->shape, p); }