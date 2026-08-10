#ifndef CAMERA_H
#define CAMERA_H
#include "base.h"

typedef struct Camera Camera;

Camera *newRegularCamera(int width, int height, int depth);

void destroyRegularCamera(Camera *camera);

ReversedRay getInitRay(Camera *camera, Real x, Real y, Real wavelength);

#endif