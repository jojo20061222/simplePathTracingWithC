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

Real getObjectInsideSigmaS(Object *self, Real lambda) {
    return getInterfaceInsideSigmaS(self->interface, lambda);
}

Vector sampleObjectInsideNextDirection(Object *self, Vector currentDirection, MT19937 *mt) {
    return sampleInterfaceInsideNextDirection(self->interface, currentDirection, mt);
}

Complex getObjectInsideRefractiveIndex(Object *self, Real lambda) {
    return getInterfaceInsideRefractiveIndex(self->interface, lambda);
}

Complex getObjectOutsideRefractiveIndex(Object *self, Real lambda) {
    return getInterfaceInsideRefractiveIndex(self->interface, lambda);
}

Real getObjectRoughness(Object *self) { return getInterfaceRoughness(self->interface); }

bool isObjectInsideMetallic(Object *self) { return isInterfaceInsideMetallic(self->interface); }

bool isObjectInsideFullyTransparent(Object *self) {
    return isInterfaceInsideFullyTransparent(self->interface);
}

bool isObjectInsideTransparent(Object *self) {
    return isInterfaceInsideTransparent(self->interface);
}

bool isObjectOutsideHalfTransparent()

bool isObjectInsideHalfTransparent(Object *self) {
    return isInterfaceInsideHalfTransparent(self->interface);
}

Real getObjectInsideBaseColor(Object *self, Real lambda) {
    return getInterfaceInsideBaseColor(self->interface, lambda);
}

Vector getNormalVector(Object *self, Point p) { return NormalVectorOfPoint(self->shape, p); }