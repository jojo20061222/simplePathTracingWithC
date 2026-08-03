#include "transform.h"
#include "math.h"

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

Vector moveBy(Vector v, Vector displacement) {
    return vecAdd(v, displacement);
}