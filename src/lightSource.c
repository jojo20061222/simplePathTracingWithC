#include "lightSource.h"

CollideLightSourceResult collideLightSource(const LightSource *source, const ReversedRay *ray) {
    CollideResult result = source->shape->detectCollide(source->shape, ray);
    return (CollideLightSourceResult){result.collide, result.t, source->intensity};
}