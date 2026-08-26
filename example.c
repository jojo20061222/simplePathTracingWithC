#include "include/pathTracing.h"

Real strongWhiteLight(Real wavelength) { return 3 * whiteLightBase(wavelength); }

static const int screenWidth = 720;
static const int screenHeight = 480;

int main() {
    static const Real width = 720;
    static const Real height = 720.0 * screenHeight / screenWidth;
    Camera *camera = newRegularCamera(screenWidth, screenHeight, width * 0.2, width * 0.8,
                                      height * 0.36, height * 0.96, -1.0 / 3.0 * height, width);
    Frame **frames = newDynamicArray(0, Frame *);
    Shape **shapes = newDynamicArray(0, Shape *);
    Texture **textures = newDynamicArray(0, Texture *);
    Object **objects = newDynamicArray(0, Object *);
    LightSource **sources = newDynamicArray(0, LightSource *);
    Interface **interfaces = newDynamicArray(0, Interface *);

    Texture *air = newAir();
    pushBack(&textures, air, Texture *);
    Texture *whitePlastic = newWhitePlastic();
    pushBack(&textures, whitePlastic, Texture *);
    Texture *pinkMarble = newPinkMarble();
    pushBack(&textures, pinkMarble, Texture *);
    Texture *diamond = newDiamond();
    pushBack(&textures, diamond, Texture *);

    Real startx = 0, endx = width;
    Real starty = height / 3, endy = height * 1.5;
    Real pz = -2.0 / 3.0 * width;
    Real bz = -1.0 / 3.0 * width;

    Real radius = width / 10;

    // groud
    Point *groundPoints = newDynamicArray(4, Point);
    groundPoints[0] = (Point){endx, starty, bz};
    groundPoints[1] = (Point){endx, starty, pz};
    groundPoints[2] = (Point){startx, starty, pz};
    groundPoints[3] = (Point){startx, starty, bz};
    Shape *groundShape = newConvexPolygon(groundPoints);
    pushBack(&shapes, groundShape, Shape *);
    freeDynamicArray(groundPoints);
    Interface *groundInterface = newInterface(whitePlastic, air, 1);
    pushBack(&interfaces, groundInterface, Interface *);
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

    Point showing = {(width / 2) - radius * 2.5, height / 2, pz + 2 * radius};
    Shape *marbleSph = newSphere(showing, radius);
    pushBack(&shapes, marbleSph, Shape *);
    Interface *marbleInterface = newInterface(pinkMarble, air, 0);
    pushBack(&interfaces, marbleInterface, Interface *);
    Object *rayleighBall = newObject(marbleSph, marbleInterface);
    pushBack(&objects, rayleighBall, Object *);

    Point diamondShowing = {width / 2, height / 2, pz + 2 * radius};
    Frame *diamondFrame = newFrame("src/static/diamond.obj");
    pushBack(&frames, diamondFrame, Frame *);
    RegularTransformPara *para = newRegularTransformPara(radius, 60, 0, 0, diamondShowing);
    Shape *diamondShape = newGroupFromFrame(diamondFrame, regularTransform, para);
    pushBack(&shapes, diamondShape, Shape *);
    destroyRegulartransformPara(para);
    Interface *diamondInterface = newInterface(diamond, air, 0);
    pushBack(&interfaces, diamondInterface, Interface *);
    Object *diamondObject = newObject(diamondShape, diamondInterface);
    pushBack(&objects, diamondObject, Object *);

    Point sapphireShowing = {width / 2 + radius * 2, height / 2, pz + 2 * radius};
    Frame *sapphireFrame = newFrame("src/static/sapphire.obj");
    pushBack(&frames, sapphireFrame, Frame *);
    RegularTransformPara *para = newRegularTransformPara(radius, 60, 0, 0, sapphireShowing);
    Shape *sapphireShape = newGroupFromFrame(sapphireFrame, regularTransform, para);
    pushBack(&shapes, sapphireShape, Shape *);
    destroyRegulartransformPara(para);
    Interface *sapphireInterface = newInterface(sapphire, air, 0);
    pushBack(&interfaces, sapphireInterface, Interface *);
    Object *sapphireObject = newObject(diamondShape, diamondInterface);
    pushBack(&objects, diamondObject, Object *);

    Real lightRadius = width / 10;

    // light
    Shape *lightSph = newSphere((Point){0, height, showing.z - radius}, lightRadius);
    pushBack(&shapes, lightSph, Shape *);
    LightSource light = newLightSource(lightSph, strongWhiteLight);
    pushBack(&sources, &light, LightSource *);

    renderToWindow("Aerogel showing frame", objects, sources, camera, screenWidth, screenHeight,
                   11);

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

    for (int i = 0; i < getSize(interfaces); i++) {
        destroyInterface(interfaces[i]);
    }
    freeDynamicArray(interfaces);

    return 0;
}