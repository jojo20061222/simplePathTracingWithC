#include "camera.h"
#include <stdlib.h>

typedef struct Camera {
    ReversedRay (*mapFunction)(struct Camera *self, Real x, Real y, Real wavelength);
    void *data;
} Camera;

Camera *newCamera(ReversedRay (*mapFunction)(Camera *, Real, Real, Real), void *data) {
    Camera *cam = malloc(sizeof(*cam));
    *cam = (Camera){mapFunction, data};
    return cam;
}

typedef struct {
    Real width, height;
    Real startX, endX;
    Real startY, endY;
    Real depth;
} RegularCameraData;

ReversedRay regularCameraMapFucntion(Camera *camera, Real x, Real y, Real wavelength) {
    RegularCameraData *data = camera->data;
    Point start = {(Real)data->width / 2, (Real)data->height / 2, (Real)data->depth};
    Point direction = vecMinus((Vector){x, y, 0}, start);
    // generate a new ray of wavelength 0
    return newRay(start, direction, wavelength);
}

// generate a new camera that maps x, y on screen to range [[startX, endX], [startY, endY], depth]
Camera *newRegularCamera(int width, int height, Real startX, Real endX, Real startY, Real endY, Real depth) {
    RegularCameraData *p = malloc(sizeof(*p));
    *p = (RegularCameraData){};
    Camera *camera = malloc(sizeof(*camera));
    *camera = (Camera){regularCameraMapFucntion, p};
    return camera;
}

void destroyRegularCamera(Camera *camera) {
    free(camera->data);
    free(camera);
}

ReversedRay getInitRay(Camera *camera, Real x, Real y, Real wavelength) {
    return camera->mapFunction(camera, x, y, wavelength);
}