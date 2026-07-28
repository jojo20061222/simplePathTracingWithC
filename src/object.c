#include "object.h"

Object newObject(Shape *shape, Texture texture) {
    return (Object){shape, texture};
}

CollideResult collideObject(Object *obj, ReversedRay *ray) {
    return obj->shape->detectCollide(obj->shape, ray);
}