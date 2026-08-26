#include "include/pathTracing.h"

Real strongWhiteLight(Real wavelength) { return 1 * whiteLightBase(wavelength); }

int main() {
    static const int width = 540;
    static const int height = 360;
    Camera *camera =
        newRegularCamera(width, height, (Real)width * 0.35, (Real)width * 0.65, (Real)height * 0.35,
                         (Real)height * 0.65, -1.0 / 3.0 * (Real)height, width);
    Frame **frames = newDynamicArray(0, Frame *);
    Shape **shapes = newDynamicArray(0, Shape *);
    Texture **textures = newDynamicArray(0, Texture *);
    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

    Texture *air = newAir();
    pushBack(&textures, air, Texture *);
    Texture *rayleigh = newAerogel();
    pushBack(&textures, rayleigh, Texture *);

    Real startx = 0, endx = width;
    Real starty = (Real)height / 3, endy = height;
    Real pz = -(Real)2 / (Real)3 * width;
    Real bz = -(Real)1 / (Real)3 * width;

    Real radius = (Real)width / 10;
    Point showing = {(Real)width / 2, (Real)height / 2, pz + 2 * radius};

    Shape *rayleighSph = newSphere(showing, radius);
    pushBack(&shapes, rayleighSph, Shape *);
    Interface *rayleighInterface = newInterface(rayleigh, air, 0.7);
    Object *rayleighBall = newObject(rayleighSph, rayleighInterface);
    pushBack(&objects, rayleighBall, Object *);

    Real lightRadius = (Real)width / 40;

    // light
    Shape *lightSph =
        newSphere((Point){showing.x+ radius * 2, showing.y, showing.z}, lightRadius);
    pushBack(&shapes, lightSph, Shape *);
    LightSource light = newLightSource(lightSph, strongWhiteLight);
    pushBack(&sources, &light, LightSource *);

    renderToWindow("Aerogel showing frame", objects, sources, camera, width, height, 11);

    destroyRegularCamera(camera);

    for (int i = 0; i < getSize(frames); i++) {
        destroyFrame(frames[i]);
    }

    for (int i = 0; i < getSize(sources); i++) {
        destroyShape(shapes[i]);
    }
    freeDynamicArray(shapes);

    for (int i = 0; i < getSize(textures); i++) {
        destroyTexture(textures[i]);
    }
    freeDynamicArray(textures);

    for (int i = 0; i < getSize(objects); i++) {
        destroyObject(objects[i]);
    }
    freeDynamicArray(objects);

    return 0;
}