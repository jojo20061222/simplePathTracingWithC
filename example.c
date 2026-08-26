#include "include/pathTracing.h"

Real strongWhiteLight(Real wavelength) { return 3 * whiteLightBase(wavelength); }

static const int screenWidth = 1080;
static const int screenHeight = 720;

int main() {
    static const Real width = 720;
    static const Real height = 480;
    Camera *camera =
        newRegularCamera(screenWidth, screenHeight, width * 0.35, width * 0.65, height * 0.35,
                         height * 0.65, -1.0 / 3.0 * height, width);
    Frame **frames = newDynamicArray(0, Frame *);
    Shape **shapes = newDynamicArray(0, Shape *);
    Texture **textures = newDynamicArray(0, Texture *);
    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);

    Texture *air = newAir();
    pushBack(&textures, air, Texture *);
    Texture *aerogel = newAerogel();
    pushBack(&textures, aerogel, Texture *);
    Texture *whiteReflector = newWhitePlastic();
    pushBack(&textures, whiteReflector, Texture *);
    Texture *pinkMarble = newPinkMarble();
    pushBack(&textures, pinkMarble, Texture *);

    Real startx = 0, endx = width;
    Real starty = height / 3, endy = height;
    Real pz = -2.0 / 3.0 * width;
    Real bz = -1.0 / 3.0 * width;

    Real radius = width / 10;
    Point showing = {width / 2, height / 2, pz + 2 * radius};

    // groud
    Point *groundPoints = newDynamicArray(4, Point);
    groundPoints[0] = (Point){endx, starty, bz};
    groundPoints[1] = (Point){endx, starty, pz};
    groundPoints[2] = (Point){startx, starty, pz};
    groundPoints[3] = (Point){startx, starty, bz};
    Shape *groundShape = newConvexPolygon(groundPoints);
    pushBack(&shapes, groundShape, Shape *);
    freeDynamicArray(groundPoints);
    Interface *groundInterface = newInterface(whiteReflector, air, 1);
    Object *ground = newObject(groundShape, groundInterface);
    pushBack(&objects, ground, Object *);

    // back
    Point *backPoints = newDynamicArray(4, Point);
    backPoints[0] = (Point){endx, starty, pz};
    backPoints[1] = (Point){endx, endy, pz};
    backPoints[2] = (Point){startx, endy, pz};
    backPoints[3] = (Point){startx, starty, pz};
    Shape *backShape = newConvexPolygon(backPoints);
    freeDynamicArray(backPoints);
    Object *backWall = newObject(backShape, groundInterface);
    pushBack(&objects, backWall, Object *);

    Shape *rayleighSph = newSphere(showing, radius);
    pushBack(&shapes, rayleighSph, Shape *);
    Interface *rayleighInterface = newInterface(pinkMarble, air, 0);
    Object *rayleighBall = newObject(rayleighSph, rayleighInterface);
    pushBack(&objects, rayleighBall, Object *);

    Real lightRadius = width / 10;

    // light
    Shape *lightSph =
        newSphere((Point){0, height * 2, showing.z - radius * 2}, lightRadius);
    pushBack(&shapes, lightSph, Shape *);
    LightSource light = newLightSource(lightSph, strongWhiteLight);
    pushBack(&sources, &light, LightSource *);

    renderToWindow("Aerogel showing frame", objects, sources, camera, screenWidth, screenHeight, 11);

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