#include "object.h"

Object newObject(Shape *shape, Texture texture) {
    return (Object){shape, texture};
}

CollideResult collideObject(Object *obj, ReversedRay *ray) {
    return detectCollide(obj->shape, ray);
}