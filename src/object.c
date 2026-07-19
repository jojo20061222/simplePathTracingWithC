#include "object.h"

CollideColorResult collideObject(const struct _Object *self, const ReversedRay *ray) {
    CollideResult collideShapeRes = self->shape->detectCollide(self->shape, ray);
    return (CollideColorResult){collideShapeRes.collide, collideShapeRes.t, self->texture.reflectivity(ray->waveLength)};
}

Object newObject(Shape *shape, Texture texture) {
    return (Object){shape, texture};
}