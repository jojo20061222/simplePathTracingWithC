#include "transform.h"
#include "math.h"
#include <stdlib.h>

typedef struct RegularTransformPara {
    Real scalingFactor;
    Real rotateXAngle, rotateYAngle, rotateZAngle;
    Vector destination;
} RegularTransformPara;

Vector scaling(Vector v, Real scaleFactor) { return vecMultiply(scaleFactor, v); }

Vector rotateAboutXAxis(Vector v, Real degree) {
    // x remains
    Real x = v.x;
    Real theta = degree / 180 * M_PI;
    Real cosTheta = cos(theta);
    Real sinTheta = sin(theta);
    Real y = cosTheta * v.y - sinTheta * v.z;
    Real z = sinTheta * v.y + cosTheta * v.z;

    return (Vector){x, y, z};
}

Vector rotateAboutYAxis(Vector v, Real degree) {
    // y remains
    Real y = v.y;
    Real theta = degree / 180 * M_PI;
    Real cosTheta = cos(theta);
    Real sinTheta = sin(theta);
    Real x = cosTheta * v.x - sinTheta * v.z;
    Real z = sinTheta * v.x + cosTheta * v.z;

    return (Vector){x, y, z};
}

Vector rotateAboutZAxis(Vector v, Real degree) {
    // z remains
    Real z = v.z;
    Real theta = degree / 180 * M_PI;
    Real cosTheta = cos(theta);
    Real sinTheta = sin(theta);
    Real x = cosTheta * v.x - sinTheta * v.y;
    Real y = sinTheta * v.x + cosTheta * v.y;

    return (Vector){x, y, z};
}

Vector moveBy(Vector v, Vector displacement) { return vecAdd(v, displacement); }

Vector regularTransform(Vector v, void *para) {
    RegularTransformPara *rtpara = para;
    v = scaling(v, rtpara->scalingFactor);
    v = rotateAboutXAxis(v, rtpara->rotateXAngle);
    v = rotateAboutYAxis(v, rtpara->rotateYAngle);
    v = rotateAboutZAxis(v, rtpara->rotateZAngle);
    v = moveBy(v, rtpara->destination);
    return v;
}

RegularTransformPara *newRegularTransformPara(Real scalingFactor, Real rotateXAngle,
                                              Real rotateYAngle, Real rotateZAngle,
                                              Vector destination) {
    RegularTransformPara *result = malloc(sizeof(*result));
    if (result == NULL) {
        fprintf(stderr, "malloc regular transform data failed\n");
        exit(1);
    }
    *result = (RegularTransformPara){scalingFactor, rotateXAngle, rotateYAngle, rotateZAngle,
                                     destination};
    return result;
}

void destroyRegulartransformPara(RegularTransformPara *para) { free(para); }