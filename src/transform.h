#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "base.h"

// since users may want to do their own transform, we left a void *
typedef Vector (*Transform)(Vector, void *);

typedef struct RegularTransformPara RegularTransformPara;

RegularTransformPara *newRegularTransformPara(double scalingFactor, double rotateXAngle,
                                              double rotateYAngle, double rotateZAngle,
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
Vector scaling(Vector v, double scaleFactor); // scaling about the origin
Vector rotateAboutXAxis(Vector v, double degree);
Vector rotateAboutYAxis(Vector v, double degree);
Vector rotateAboutZAxis(Vector v, double degree);

// after doing transformation above than move it to the right place
Vector moveBy(Vector v, Vector displacement);

#endif