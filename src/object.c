#include "object.h"

CollideColorResult collideObject(const struct _Object *self, const ReversedRay *ray) {
    CollideResult collideShapeRes = self->shape->detectCollide(self->shape, ray);
    return (CollideColorResult){collideShapeRes.collide, collideShapeRes.t, self->reflectivity(ray->waveLength)};
}

Object newObject(Shape *shape, double(*reflectivity)(double), double diffuseFactor) {
    return (Object){shape, reflectivity, diffuseFactor};
}