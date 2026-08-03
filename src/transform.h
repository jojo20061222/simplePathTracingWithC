#include "base.h"

typedef Vector (*Transform)(Vector);

// Provide some base transformation
Vector scaling(Vector v, double scaleFactor); // scaling about the origin
Vector rotateAboutXAxis(Vector v, double degree);
Vector rotateAboutYAxis(Vector v, double degree);
Vector rotateAboutZAxis(Vector v, double degree);

// after doing transformation above than move it to the right place
Vector moveBy(Vector v, Vector displacement);