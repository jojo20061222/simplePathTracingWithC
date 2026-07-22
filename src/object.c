#include "object.h"

Object newObject(Shape *shape, Texture texture) {
    return (Object){shape, texture};
}

CollideResult collideObject(const Object *obj, const ReversedRay *ray) {
    return obj->shape->detectCollide(obj->shape, ray);
}