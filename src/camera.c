#include "camera.h"
#include <stdlib.h>


typedef struct Camera {
    ReversedRay (*mapFunction)(struct Camera *self, Real x, Real y, Real wavelength);
    void *data;
} Camera;

// map function will map x, y on the screen into real world
// data is data that mapfunction might use
Camera *newCamera(ReversedRay (*mapFunction)(Camera *, Real, Real, Real), void *data) {
    Camera *cam = malloc(sizeof(*cam));
    *cam = (Camera){mapFunction, data};
    return cam;
}

typedef struct {
    Real width, height;
    Real startX, endX;
    Real startY, endY;
    Real z;
    Point start;
} RegularCameraData;


ReversedRay regularCameraMapFucntion(Camera *camera, Real x, Real y, Real wavelength) {
    RegularCameraData *data = camera->data;
    // linearly map the x, y on screen to reality
    Real convertedX = lerp(0, data->width, x, data->startX, data->endX);
    Real convertedY = lerp(0, data->height, y, data->startY, data->endY);
    Point direction = vecMinus((Vector){convertedX, convertedY, data->z}, data->start);
    return newRay(data->start, direction, wavelength);
}

/*
 * @brief: generate a new camera that maps x, y on screen to range [[startX, endX], [startY, endY],
 * z] with observer at the middle and distant from the sensor "depth"
 * @param:
 * - width, height: the size of screen
 * - startX, endX ... z : the actual cooredinates in the space
 * - depth: the distance between sensor and observer
 */
Camera *newRegularCamera(int width, int height, Real startX, Real endX, Real startY, Real endY,
                         Real z, Real depth) {
    RegularCameraData *p = malloc(sizeof(*p));
    Point start = {(startX + endX) / 2, (startY + endY) / 2, z + depth};
    *p = (RegularCameraData){(Real)width, (Real)height, startX, endX, startY, endY, z, start};
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