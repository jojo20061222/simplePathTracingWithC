#ifndef CAMERA_H
#define CAMERA_H
#include "base.h"

typedef struct Camera Camera;

Camera *newRegularCamera(int width, int height, Real startX, Real endX, Real startY, Real endY,
                         Real z, Real depth);

void destroyRegularCamera(Camera *camera);

ReversedRay getInitRay(Camera *camera, Real x, Real y, Real wavelength);

#endif