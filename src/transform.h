#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "base.h"

// since users may want to do their own transform, we left a void *
typedef Vector (*Transform)(Vector, void *);

typedef struct RegularTransformPara RegularTransformPara;

RegularTransformPara *newRegularTransformPara(Real scalingFactor, Real rotateXAngle,
                                              Real rotateYAngle, Real rotateZAngle,
                                              Vector destination);
void destroyRegulartransformPara(RegularTransformPara *);

// regular transform will do the following thing:
// scale by scaling factor
// rotate about x axis
// rotate about y axis
// rotate about z axis
// move to destination
Vector regularTransform(Vector v, void *);

// Provide some base transformation
Vector scaling(Vector v, Real scaleFactor); // scaling about the origin
Vector rotateAboutXAxis(Vector v, Real degree);
Vector rotateAboutYAxis(Vector v, Real degree);
Vector rotateAboutZAxis(Vector v, Real degree);

// after doing transformation above than move it to the right place
Vector moveBy(Vector v, Vector displacement);

#endif