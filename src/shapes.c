#include "shapes.h"
#include "base.h"
#include "dynamicArray.h"
#include "random.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*
 * @author seaofvoices
 * @brief Gives the definition of shape and default shapes.
 */

typedef struct Shape {
    CollideResult (*detectCollide)(struct Shape *, ReversedRay *);
    void (*destroySelf)(struct Shape *);
    void (*randomPoint)(struct Shape *, Real *PDF, Point *point, MT19937 *mt);
} Shape;

typedef struct Triangle {
    Shape shape;
    Point p1, p2, p3;
    Vector normal;
} Triangle;

typedef struct Sphere {
    Shape shape;
    Point center;
    Real radius;
} Sphere;

typedef struct ConvexPolygon {
    Shape shape;
    Point *vertexes;
    Vector normal;
} ConvexPolygon;

typedef struct Group {
    Shape shape;
    Shape **elements;
} Group;

static inline CollideResult getCollideResult(bool collide, Real t, Vector normalVector) {
    return (CollideResult){collide, t, normalVector};
}

static inline CollideResult notCollide() {
    return (CollideResult){false, INFINITY, (Vector){0, 0, 0}};
}

static inline Shape
getShape(CollideResult (*detectCollide)(Shape *, ReversedRay *), void (*destroySelf)(Shape *),
         void (*randomPoint)(struct Shape *, Real *PDF, Point *point, MT19937 *mt)) {
    return (Shape){detectCollide, destroySelf, randomPoint};
}

static CollideResult detectTriangleCollide(Shape *self, ReversedRay *ray) {
    CollideResult result = {false, INFINITY};
    /*
     * Möller-Trumbore intersection algorithm
     * Parametrize the triangle with T(u, v) and the points lies in the triangle can be expressed as
     * T(u, v) = uA + vB + P, u + v <= 1, 0 <= u, v <= 1 where A, B, P represent two side vector and
     * one vertex, respectively.
     * Then, let line be L(t) = O + tD we have O + tD = uA + vB + P. Since there are three unknowns
     * and three equations t, u, v can thus be solved.
     */

    Triangle *tri = (Triangle *)self;
    Vector A = vecMinus(tri->p2, tri->p1);
    Vector B = vecMinus(tri->p3, tri->p1);
    Vector W = vecMinus(ray->start, tri->p1); // let W = O - P
    Vector D = ray->direction;

    /*
     * O.x + D.x t = A.x u + B.x v + P.x => A.x u + B.x v - D.x t = O.x - P.x = W.x
     * Similarly we have other two equations, and thus Delta
     *     | A.x B.x -D.x |       | W.x B.x -D.x |
     * D = | A.y B.y -D.y |, Du = | W.y B.y -D.y |, etc.
     *     | A.z B.z -D.z |       | W.z B.z -D.z |
     */

    Real delta = A.x * B.y * (-D.z) + A.z * B.x * (-D.y) + A.y * B.z * (-D.x) - A.z * B.y * (-D.x) -
                 A.x * B.z * (-D.y) - A.y * B.x * (-D.z);
    if (rabs(delta) < error) return result;

    Real deltaU = W.x * B.y * (-D.z) + W.z * B.x * (-D.y) + W.y * B.z * (-D.x) -
                  W.z * B.y * (-D.x) - W.x * B.z * (-D.y) - W.y * B.x * (-D.z);
    Real u = deltaU / delta;
    if (u < 0 || u > 1) return result;

    Real deltaV = A.x * W.y * (-D.z) + A.z * W.x * (-D.y) + A.y * W.z * (-D.x) -
                  A.z * W.y * (-D.x) - A.x * W.z * (-D.y) - A.y * W.x * (-D.z);
    Real v = deltaV / delta;
    if (v < 0 || u > 1 || u + v > 1) return result;

    Real deltaT = A.x * B.y * W.z + A.z * B.x * W.y + A.y * B.z * W.x - A.z * B.y * W.x -
                  A.x * B.z * W.y - A.y * B.x * W.z;
    Real t = deltaT / delta;
    if (t < 0) return result;

    result = getCollideResult(true, t, tri->normal);

    return result;
}

static void destroyTriangle(Shape *self) { free(self); }

Shape *newTriangle(Point p1, Point p2, Point p3) {
    Triangle *tri = malloc(sizeof(*tri));
    if (tri == NULL) {
        fprintf(stderr, "malloc tri failed\n");
        exit(1);
    }
    tri->shape = getShape(detectTriangleCollide, destroyTriangle, NULL);
    tri->p1 = p1;
    tri->p2 = p2;
    tri->p3 = p3;
    tri->normal = unitVector(vecCross(vecMinus(p2, p1), vecMinus(p3, p1)));
    return (Shape *)tri;
}

// Returns outward normal vector
static inline Vector normalVectorOfSphere(Sphere *sph, Point p) {
    return unitVector(vecMinus(p, sph->center));
}

static CollideResult detectSphereCollide(Shape *self, ReversedRay *ray) {
    /*
     * Some math.
     * Let R(t) = O + tD = <a + tx, b + ty, c + tz>, center of sphere C = <e, f, g>
     * <a + tx, b + ty, c + tz> - <e, f, g> = <a - e + tx, b - f + ty, c - g + tz>
     * Hence we solve for minimum of (a - e + tx)^2 + (b - f + ty)^2 + (c - g + tz)^2 = r^2
     * Let W = O - C = <o, p, q>
     * then (x^2 + y^2 + z^2)t^2 + 2(ox + py + qz)t + (o^2 + p^2 + q^2 - r^2) = 0
     */

    Sphere *sph = (Sphere *)self;

    Vector W = vecMinus(ray->start, sph->center);
    Real A = vecDot(ray->direction, ray->direction);
    Real B = 2 * vecDot(ray->direction, W);
    Real C = vecDot(W, W) - sph->radius * sph->radius;
    Real Delta = B * B - 4 * A * C;
    if (Delta < 0) {
        return notCollide();
    }
    Real t1 = (-B - sqrt(Delta)) / (2 * A), t2 = (-B + sqrt(Delta)) / (2 * A);
    if (t1 < 0 && t2 < 0) {
        return notCollide();
    }
    Real t = t1 < 0 ? t2 : t1;
    Point collidePoint = vecAdd(ray->start, vecMultiply(t, ray->direction));
    return getCollideResult(true, t, normalVectorOfSphere(sph, collidePoint));
}

static void destroySphere(Shape *self) { free(self); }

// choose a ramdom point on sphere
void randomPointOnSphere(Shape *self, Real *PDF, Point *point, MT19937 *mt) {
    Sphere *sph = (Sphere *)self;
    *PDF = 1 / (4 * M_PI * sph->radius * sph->radius); // 1 / (4 pi r^2)
    Real u = randomReal(mt), v = randomReal(mt);
    Real theta = 2 * M_PI * u;
    Real z = 1 - 2 * v;
    Real sinPhi = sqrt(1 - z * z);
    Real x = sinPhi * rcos(theta), y = sinPhi * rsin(theta);
    *point = (Point){x, y, z}; // <sqrt(1 - u^2) * cos(theta), sqrt(1 - u^2) * sin(theta), u>
}

Shape *newSphere(Point center, Real radius) {
    Sphere *sph = malloc(sizeof(*sph));
    if (sph == NULL) {
        fprintf(stderr, "malloc sph failed\n");
        exit(1);
    }
    sph->shape = getShape(detectSphereCollide, destroySphere, randomPointOnSphere);
    sph->center = center;
    sph->radius = radius;
    return (Shape *)sph;
}

static CollideResult detectConvexPolygonCollide(Shape *self, ReversedRay *ray) {
    ConvexPolygon *cp = (ConvexPolygon *)self;
    int vertexesCount = getSize(cp->vertexes);
    // First we get the plane of the polygon
    // get the normal vector
    assert(vertexesCount >= 3);
    Vector N = cp->normal;

    // we want to solve the intersection of the plane and the ray
    // the plane can be written as
    // N = <a, b, c>
    // ray.direction = D = <d, e, f>
    // ray.start = O = <p, q, r>
    // N dot <x, y, z> = (ax_0 + by_0 + b_z0) = C
    // => N dot (O + tD) = C
    // N dot O + t (N dot D) = C
    // (C - N dot O) / (N dot D) = t
    // The question is, if N dot D nears 0
    // That the ray just lies on the plane or parallel to the plane
    // we simply count this situation as no collision
    Vector D = ray->direction;
    Vector O = ray->start;
    Real C = vecDot(N, cp->vertexes[0]);
    Real NdotD = vecDot(N, D);
    if (rabs(NdotD) < error) {
        return notCollide();
    }
    Real t = (C - vecDot(N, O)) / NdotD;
    if (t <= error) {
        return notCollide();
    }
    Point P = vecAdd(O, vecMultiply(t, D));
    // Now we have converted this problem into if a point is in the convex polygon
    for (int i = 0; i < vertexesCount; i++) {
        // Since it's an convex polygon, we see if the point is on the left side of every edge
        Point P1 = cp->vertexes[i];
        Point P2 = cp->vertexes[(i + 1) % vertexesCount];
        // E is the vector from start point to intersection P
        Vector E = vecMinus(P, P1);
        // E1 is the polygon's ith edge
        Vector E1 = vecMinus(P2, P1);
        Vector crossResult = vecCross(E1, E);
        if (vecDot(crossResult, N) < 0) {
            return notCollide();
        }
    }
    return getCollideResult(true, t, cp->normal);
}

static void destroyConvexPolygon(Shape *self) {
    ConvexPolygon *cp = (ConvexPolygon *)self;
    freeDynamicArray(cp->vertexes);
    free(self);
}

Shape *newConvexPolygon(Point *vertexes) {
    ConvexPolygon *cp = malloc(sizeof(*cp));
    if (cp == NULL) {
        fprintf(stderr, "malloc cp failed\n");
        exit(1);
    }
    cp->shape = getShape(detectConvexPolygonCollide, destroyConvexPolygon, NULL);
    cp->vertexes = newDynamicArray(getSize(vertexes), Point);

    int vertexesCount = getSize(cp->vertexes);
    assert(vertexesCount >= 3);
    memcpy(cp->vertexes, vertexes, sizeof(Point) * vertexesCount);

    Vector N = {NAN, NAN, NAN}; // theoretically N should not be this after the loop
    // we iterate the vertexes and find the vertexes that are not in a single line
    for (int i = 0; i < vertexesCount; i++) {
        Point p1 = cp->vertexes[i];
        Point p2 = cp->vertexes[(i + 1) % vertexesCount];
        Point p3 = cp->vertexes[(i + 2) % vertexesCount];
        Vector e1 = vecMinus(p2, p1);
        Vector e2 = vecMinus(p3, p1);
        N = vecCross(e1, e2);
        if (lengthOf(N) > 1e-5) break;
    }
    cp->normal = unitVector(N);

    return (Shape *)cp;
}

CollideResult detectGroupCollide(Shape *self, ReversedRay *ray) {
    Group *group = (Group *)self;
    int shapesCount = getSize(group->elements);
    CollideResult firstCollideObjectResult = notCollide();
    for (int i = 0; i < shapesCount; i++) {
        CollideResult curResult = group->elements[i]->detectCollide(group->elements[i], ray);
        if (curResult.distance <= firstCollideObjectResult.distance) {
            firstCollideObjectResult = curResult;
        }
    }
    return firstCollideObjectResult;
}

void destroyGroup(Shape *self) {
    Group *group = (Group *)self;
    int shapesCount = getSize(group->elements);
    for (int i = 0; i < shapesCount; i++) {
        group->elements[i]->destroySelf(group->elements[i]);
    }
    freeDynamicArray(group->elements);
    free(group);
}

Shape *newGroup(Shape **elements) {
    Group *group = malloc(sizeof(*group));
    if (group == NULL) {
        fprintf(stderr, "malloc group failed\n");
        exit(1);
    }
    group->elements = newDynamicArray(getSize(elements), Shape *);
    memcpy(group->elements, elements, sizeof(Shape *) * getSize(elements));
    group->shape = getShape(detectGroupCollide, destroyGroup, NULL);
    return (Shape *)group;
}

// provide a uniform api to others to detect collide
CollideResult detectCollide(Shape *self, ReversedRay *ray) {
    return self->detectCollide(self, ray);
}

// provide a uniform apt to others to destroy a shape
void destroyShape(Shape *self) { self->destroySelf(self); }