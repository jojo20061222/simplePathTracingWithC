#include "object.h"
#include <stdlib.h>

typedef struct Object {
    Shape *shape;
    Texture texture;
} Object;

Object *newObject(Shape *shape, Texture texture) {
    Object *obj = malloc(sizeof(*obj));
    obj->shape = shape;
    obj->texture = texture;
    return obj;
}

CollideResult collideObject(Object *obj, ReversedRay *ray) {
    return detectCollide(obj->shape, ray);
}

Complex getRefractiveIndexOutside(Object *self, double lambda) {
    return self->texture.refractiveIndexOutside(lambda);
}

Complex getRefractiveIndexInside(Object *self, double lambda) {
    return self->texture.refractiveIndexInside(lambda);
}

double getRoughness(Object *self) {
    return self->texture.roughness;
}

bool getMetalic(Object *self) {
    return self->texture.metalic;
}

bool getTransparent(Object *self) {
    return self->texture.transparent;
}

double getBaseColor(Object *self, double lambda) {
    return self->texture.baseColor(lambda);
}