#include "include/pathTracing.h"

static const int screenWidth = 720;
static const int screenHeight = 480;

LightIntensity coldLightBase(Real lambda) {
    return getBlackbodyIntensity(lambda, 9500) / 158005180313225.187500;
}

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
    Texture *sapphire = newSapphire(5e-3);
    pushBack(&textures, sapphire, Texture *);
    Texture *gold = newGold();
    pushBack(&textures, gold, Texture *);
    Texture *glass = newGlass();
    pushBack(&textures, glass, Texture *);
    Texture *aerogel = newAerogel();
    pushBack(&textures, aerogel, Texture *);

    Real startx = -width, endx = 2 * width;
    Real starty = height / 3, endy = height * 1.5;
    Real pz = -2.0 / 3.0 * width;
    Real bz = 1.0 / 3.0 * width;

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

    Point marbleShowing = {(width / 2) - radius * 2.5, height / 2, pz + 2 * radius};
    Shape *marbleSph = newSphere(marbleShowing, radius);
    pushBack(&shapes, marbleSph, Shape *);
    Interface *marbleInterface = newInterface(pinkMarble, air, 0);
    pushBack(&interfaces, marbleInterface, Interface *);
    Object *rayleighBall = newObject(marbleSph, marbleInterface);
    pushBack(&objects, rayleighBall, Object *);

    Point diamondShowing = {width / 2, height / 2, pz + 2 * radius};
    Frame *diamondFrame = newFrame("src/static/diamond.obj");
    pushBack(&frames, diamondFrame, Frame *);
    RegularTransformPara *diamondPara = newRegularTransformPara(radius, 60, 0, 0, diamondShowing);
    Shape *diamondShape = newGroupFromFrame(diamondFrame, regularTransform, diamondPara);
    pushBack(&shapes, diamondShape, Shape *);
    destroyRegulartransformPara(diamondPara);
    Interface *diamondInterface = newInterface(diamond, air, 0);
    pushBack(&interfaces, diamondInterface, Interface *);
    Object *diamondObject = newObject(diamondShape, diamondInterface);
    pushBack(&objects, diamondObject, Object *);

    Point sapphireShowing = {width / 2 + radius * 2.5, height / 2, pz + 2 * radius};
    Frame *sapphireFrame = newFrame("src/static/gem.obj");
    pushBack(&frames, sapphireFrame, Frame *);
    RegularTransformPara *sapphirePara = newRegularTransformPara(radius, 60, 0, 0, sapphireShowing);
    Shape *sapphireShape = newGroupFromFrame(sapphireFrame, regularTransform, sapphirePara);
    pushBack(&shapes, sapphireShape, Shape *);
    destroyRegulartransformPara(sapphirePara);
    Interface *sapphireInterface = newInterface(sapphire, air, 0);
    pushBack(&interfaces, sapphireInterface, Interface *);
    Object *sapphireObject = newObject(sapphireShape, sapphireInterface);
    pushBack(&objects, sapphireObject, Object *);

    Point glassShowing = {width / 2 + radius * 2.5, height / 2 + radius * 2.3, pz + 2 * radius};
    Frame *glassFrame = newFrame("src/static/cube.obj");
    pushBack(&frames, glassFrame, Frame *);
    RegularTransformPara *glassPara =
        newRegularTransformPara(radius * 0.65, 45, 45, 45, glassShowing);
    Shape *glassShape = newGroupFromFrame(glassFrame, regularTransform, glassPara);
    pushBack(&shapes, glassShape, Shape *);
    destroyRegulartransformPara(glassPara);
    Interface *glassInterface = newInterface(glass, air, 0.5);
    pushBack(&interfaces, glassInterface, Interface *);
    Object *glassObject = newObject(glassShape, glassInterface);
    pushBack(&objects, glassObject, Object *);

    Point aerogelShowing = {width / 2 - radius * 2.5, height / 2 + radius * 2.3, pz + 2 * radius};
    Frame *aerogelFrame = newFrame("src/static/cube.obj");
    pushBack(&frames, aerogelFrame, Frame *);
    RegularTransformPara *aerogelPara =
        newRegularTransformPara(radius * 0.75, 0, 0, 0, aerogelShowing);
    Shape *aerogelShape = newGroupFromFrame(aerogelFrame, regularTransform, aerogelPara);
    pushBack(&shapes, aerogelShape, Shape *);
    destroyRegulartransformPara(aerogelPara);
    Interface *aerogelInterface = newInterface(aerogel, air, 1);
    pushBack(&interfaces, aerogelInterface, Interface *);
    Object *aerogelObject = newObject(aerogelShape, aerogelInterface);
    pushBack(&objects, aerogelObject, Object *);

    Point goldShowing = {width / 2, height / 2 + radius * 2.3, pz + 2 * radius};
    Frame *goldFrame = newFrame("src/static/apple.obj");
    pushBack(&frames, goldFrame, Frame *);
    RegularTransformPara *goldPara = newRegularTransformPara(radius * 5, 0, 0, 0, goldShowing);
    Shape *goldShape = newGroupFromFrame(goldFrame, regularTransform, goldPara);
    pushBack(&shapes, goldShape, Shape *);
    destroyRegulartransformPara(goldPara);
    Interface *goldInterface = newInterface(gold, air, 0.3);
    pushBack(&interfaces, goldInterface, Interface *);
    Object *goldObject = newObject(goldShape, goldInterface);
    pushBack(&objects, goldObject, Object *);

    Real lightRadius = width / 40;

    // light
    Shape *lightSph1 =
        newSphere((Point){0, height, diamondShowing.z + radius * 2}, lightRadius * 4);
    pushBack(&shapes, lightSph1, Shape *);
    LightSource light1 = newLightSource(lightSph1, tungstenLampBase);
    pushBack(&sources, &light1, LightSource *);

    Shape *lightSph2 =
        newSphere((Point){diamondShowing.x, diamondShowing.y, pz + lightRadius}, lightRadius);
    pushBack(&shapes, lightSph2, Shape *);
    LightSource light2 = newLightSource(lightSph2, sunlightBase);
    pushBack(&sources, &light2, LightSource *);

    Shape *lightSph3 =
        newSphere((Point){width, height, diamondShowing.z + radius * 2}, lightRadius * 4);
    pushBack(&shapes, lightSph3, Shape *);
    LightSource light3 = newLightSource(lightSph3, coldLightBase);
    pushBack(&sources, &light3, LightSource *);

    Shape *lightSph4 = newSphere((Point){0, height, 0}, lightRadius * 10);
    pushBack(&shapes, lightSph4, Shape *);
    LightSource light4 = newLightSource(lightSph4, sunlightBase);
    pushBack(&sources, &light4, LightSource *);

    renderToWindow("Example Showing Frame", objects, sources, camera, screenWidth, screenHeight, 30);

    destroyRegularCamera(camera);

    for (int i = 0; i < getSize(frames); i++) {
        destroyFrame(frames[i]);
    }

    for (int i = 0; i < getSize(shapes); i++) {
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