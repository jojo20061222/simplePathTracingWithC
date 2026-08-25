#include "include/pathTracing.h"

Real strongWhiteLight(Real wavelength) { return 3 * whiteLightBase(wavelength); }

Vector sampleMarbleNextDirection(Vector input, MT19937 *mt) {
    return sampleHenyeyGreenstein(input, mt, 0.9);
}

Real getPinkMarbleSigmaS(Real lambda) {
    if (lambda < 470.0) {
        return 0.21;
    }
    else if (lambda < 530.0) {
        Real t = (lambda - 470.0) / (530.0 - 470.0);
        return lerp(0.21, 0.22, t);
    }
    else if (lambda < 650.0) {
        Real t = (lambda - 530.0) / (650.0 - 530.0);
        return lerp(0.22, 0.20, t);
    }
    else {
        return 0.20;
    }
}

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
    Texture *whitePlastic = newOpaqueDieletric(getPlasticRefractivity, pureWhite);
    pushBack(&textures, whitePlastic, Texture *);
    Texture *marble = newHalfTransparentDieletric(getPinkMarbleRefractivity, getPinkMarbleSigmaS,
                                                  sampleMarbleNextDirection);
    pushBack(&textures, marble, Texture *);

    Real startx = 0, endx = width;
    Real starty = (Real)height / 3, endy = height;
    Real pz = -(Real)2 / (Real)3 * width;
    Real bz = -(Real)1 / (Real)3 * width;

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

    Real radius = (Real)width / 10;
    Point showing = {(Real)width / 2, (Real)height / 2, pz + 2 * radius};

    Shape *marbleSph = newSphere(showing, radius);
    pushBack(&shapes, marbleSph, Shape *);
    Interface *marbleInterface = newInterface(marble, air, 0);
    Object *marbleBall = newObject(marbleSph, marbleInterface);
    pushBack(&objects, marbleBall, Object *);

    Real lightRadius = (Real)width / 40;

    // light 1: upper-left behind the marble
    Shape *lightSph1 =
        newSphere((Point){(Real)width * 0.25, (Real)height * 0.8, pz + radius}, lightRadius);
    pushBack(&shapes, lightSph1, Shape *);
    LightSource light1 = newLightSource(lightSph1, strongWhiteLight);
    pushBack(&sources, &light1, LightSource *);

    // light 2: upper-right in front of the marble
    Shape *lightSph2 =
        newSphere((Point){(Real)width * 0.75, (Real)height * 0.8, pz + 5 * radius}, lightRadius);
    pushBack(&shapes, lightSph2, Shape *);
    LightSource light2 = newLightSource(lightSph2, strongWhiteLight);
    pushBack(&sources, &light2, LightSource *);

    // light 3: directly behind the marble
    Shape *lightSph3 =
        newSphere((Point){(Real)width / 2 + lightRadius, (Real)height / 2, pz + lightRadius},
                  lightRadius * 2);
    pushBack(&shapes, lightSph3, Shape *);
    LightSource light3 = newLightSource(lightSph3, strongWhiteLight);
    pushBack(&sources, &light3, LightSource *);

    renderToWindow("Pink marble showing frame", objects, sources, camera, width, height, 11);

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