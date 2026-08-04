#include "transform.h"
#include "math.h"
#include <stdlib.h>

typedef struct RegularTransformPara {
    double scalingFactor;
    double rotateXAngle, rotateYAngle, rotateZAngle;
    Vector destination;
} RegularTransformPara;

Vector scaling(Vector v, double scaleFactor) { return vecMultiply(scaleFactor, v); }

Vector rotateAboutXAxis(Vector v, double degree) {
    // x remains
    double x = v.x;
    double theta = degree / 180 * M_PI;
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double y = cosTheta * v.y - sinTheta * v.z;
    double z = sinTheta * v.y + cosTheta * v.z;

    return (Vector){x, y, z};
}

Vector rotateAboutYAxis(Vector v, double degree) {
    // y remains
    double y = v.y;
    double theta = degree / 180 * M_PI;
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double x = cosTheta * v.x - sinTheta * v.z;
    double z = sinTheta * v.x + cosTheta * v.z;

    return (Vector){x, y, z};
}

Vector rotateAboutZAxis(Vector v, double degree) {
    // z remains
    double z = v.z;
    double theta = degree / 180 * M_PI;
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double x = cosTheta * v.x - sinTheta * v.y;
    double y = sinTheta * v.x + cosTheta * v.y;

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

RegularTransformPara *newRegularTransformPara(double scalingFactor, double rotateXAngle,
                                              double rotateYAngle, double rotateZAngle,
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

void destroyRegulartransformPara(RegularTransformPara *para) {
    free(para);
}