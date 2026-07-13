#include "object.h"

CollideColorResult collideObject(const struct _Object *self, const ReversedRay *ray) {
    CollisionResult collideShapeRes = self->shape->detectCollide(self->shape, ray);
    return (CollideColorResult){collideShapeRes.collide, collideShapeRes.t, self->color};
}

Object newObject(Shape *shape, ObjectColor color, double diffuseFactor, double specularFactor) {
    return (Object){shape, color, diffuseFactor, specularFactor};
}