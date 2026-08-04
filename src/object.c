#include "object.h"
#include <stdlib.h>

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

CollideResult collideObject(Object *obj, ReversedRay *ray) {
    return detectCollide(obj->shape, ray);
}

Complex getRefractiveIndexOutside(Object *self, double lambda) {
    return getTextureRefractiveIndexOutside(self->texture, lambda);
}

Complex getRefractiveIndexInside(Object *self, double lambda) {
    return getTextureRefractiveIndexInside(self->texture, lambda);
}

double getRoughness(Object *self) { return getTextureRoughness(self->texture); }

bool getmetallic(Object *self) { return getTexturemetallic(self->texture); }

bool getTransparent(Object *self) { return getTextureTransparent(self->texture); }

double getBaseColor(Object *self, double lambda) {
    return getTextureBaseColor(self->texture, lambda);
}