#include "object.h"
#include <stdlib.h>

/*
 * @author seaofvoices
 * @brief This file defines attributes an object should possess and provide interfaces to others.
 */

typedef struct Object {
    Shape *shape;
    Texture *texture;
} Object;

Object *newObject(Shape *shape, Texture *texture) {
    Object *obj = malloc(sizeof(*obj));
    if (obj == NULL) {
        fprintf(stderr, "malloc obj failed\n");
        exit(1);
    }
    obj->shape = shape;
    obj->texture = texture;
    return obj;
}

void destroyObject(Object *obj) { free(obj); }

CollideResult collideObject(Object *obj, ReversedRay *ray) {
    return detectCollide(obj->shape, ray);
}

Complex getRefractiveIndexOutside(Object *self, Real lambda) {
    return getTextureRefractiveIndexOutside(self->texture, lambda);
}

Complex getRefractiveIndexInside(Object *self, Real lambda) {
    return getTextureRefractiveIndexInside(self->texture, lambda);
}

Real getRoughness(Object *self) { return getTextureRoughness(self->texture); }

bool getmetallic(Object *self) { return getTexturemetallic(self->texture); }

bool getTransparent(Object *self) { return getTextureTransparent(self->texture); }

Real getBaseColor(Object *self, Real lambda) { return getTextureBaseColor(self->texture, lambda); }

Vector getNormalVector(Object *self, Point p) { return NormalVectorOfPoint(self->shape, p); }