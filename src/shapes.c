#include "dynamicArray.h"
#include "shapes.h"
#include "base.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

static CollideResult detectTriangleCollide(struct _Shape *self, ReversedRay *ray) {
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

    double delta = A.x * B.y * (-D.z) + A.z * B.x * (-D.y) + A.y * B.z * (-D.x) -
                   A.z * B.y * (-D.x) - A.x * B.z * (-D.y) - A.y * B.x * (-D.z);
    if (fabs(delta) < error) return result;

    double deltaU = W.x * B.y * (-D.z) + W.z * B.x * (-D.y) + W.y * B.z * (-D.x) -
                    W.z * B.y * (-D.x) - W.x * B.z * (-D.y) - W.y * B.x * (-D.z);
    double u = deltaU / delta;
    if (u < 0 || u > 1) return result;

    double deltaV = A.x * W.y * (-D.z) + A.z * W.x * (-D.y) + A.y * W.z * (-D.x) -
                    A.z * W.y * (-D.x) - A.x * W.z * (-D.y) - A.y * W.x * (-D.z);
    double v = deltaV / delta;
    if (v < 0 || u > 1 || u + v > 1) return result;

    double deltaT = A.x * B.y * W.z + A.z * B.x * W.y + A.y * B.z * W.x - A.z * B.y * W.x -
                    A.x * B.z * W.y - A.y * B.x * W.z;
    double t = deltaT / delta;
    if (t < 0) return result;

    result = (CollideResult){true, t};

    return result;
}

// Looks down from the top, (p1, p2, p3) in counter-clockwise order
// Then, the result of this function should return upward vector
static  Vector normalVectorOfTriangle(struct _Shape *self, Point _) {
    const Triangle *tri = (const Triangle *)self;
    return vecCross(vecMinus(tri->p2, tri->p1), vecMinus(tri->p3, tri->p1));
}

static void destroyTriangle(Shape *self) { free(self); }

Shape *newTriangle(Point p1, Point p2, Point p3) {
    Triangle *tri = malloc(sizeof(*tri));
    tri->shape = (Shape){detectTriangleCollide, normalVectorOfTriangle, destroyTriangle};
    tri->p1 = p1;
    tri->p2 = p2;
    tri->p3 = p3;
    return (Shape *)tri;
}

static CollideResult detectSphereCollide(struct _Shape *self, ReversedRay *ray) {
    /*
     * Some math.
     * Let R(t) = O + tD = <a + tx, b + ty, c + tz>, center of sphere C = <e, f, g>
     * <a + tx, b + ty, c + tz> - <e, f, g> = <a - e + tx, b - f + ty, c - g + tz>
     * Hence we solve for minimum of (a - e + tx)^2 + (b - f + ty)^2 + (c - g + tz)^2 = r^2
     * Let W = O - C = <o, p, q>
     * then (x^2 + y^2 + z^2)t^2 + 2(ox + py + qz)t + (o^2 + p^2 + q^2 - r^2) = 0
     */

    const Sphere *sphere = (const Sphere *)self;

    Vector W = vecMinus(ray->start, sphere->center);
    double A = vecDot(ray->direction, ray->direction);
    double B = 2 * vecDot(ray->direction, W);
    double C = vecDot(W, W) - sphere->radius * sphere->radius;
    double Delta = B * B - 4 * A * C;
    if (Delta < 0) return (CollideResult){false, INFINITY};
    double t1 = (-B - sqrt(Delta)) / (2 * A), t2 = (-B + sqrt(Delta)) / (2 * A);
    if (t1 < 0 && t2 < 0)
        return (CollideResult){false, INFINITY};
    else if (t1 < 0)
        return (CollideResult){true, t2};
    return (CollideResult){true, t1};
}

// Returns outward normal vector
static Vector normalVectorOfSphere(struct _Shape *self, Point p) {
    const Sphere *sph = (const Sphere *)self;
    return vecMinus(p, sph->center);
}

static void destroySphere(Shape *self) {
    free(self);
}

Shape *newSphere(Point center, double radius) {
    Sphere *sph = malloc(sizeof(*sph));
    sph->shape = (Shape){detectSphereCollide, normalVectorOfSphere, destroySphere};
    sph->center = center;
    sph->radius = radius;
    return (Shape *)sph;
}

static Vector normalVectorOfDirectedTriangle(Shape *self, Point _) {
    DirectedTriangle *tri = (DirectedTriangle *)self;
    return tri->normalVector;
}

static void destroyDirectedTriangle(Shape *self) {
    free(self);
}

Shape *newDirectedTriangle(Point p1, Point p2, Point p3, Vector normalVector) {
    DirectedTriangle *dtri = malloc(sizeof(*dtri));
    dtri->tri.shape = (Shape){detectTriangleCollide, normalVectorOfDirectedTriangle, NULL};
    dtri->normalVector = normalVector;
    return (Shape*)dtri;
}

static Vector normalVectorOfConvexPolygon(Shape *self, Point _) {
    ConvexPolygon *cp = (ConvexPolygon*)self;
    int vertexesCount = getSize(cp->vertexes);
    assert(vertexesCount >= 3);
    Point p1 = cp->vertexes[0];
    Point p2 = cp->vertexes[1];
    Point p3 = cp->vertexes[2];
    Vector e1 = vecMinus(p2, p1);
    Vector e2 = vecMinus(p3, p1);
    return vecCross(e1, e2);
}

static CollideResult collideWithConvexPolygon(Shape *self, ReversedRay *ray) {
    ConvexPolygon *cp = (ConvexPolygon *)self;
    int vertexesCount = getSize(cp->vertexes);
    // First we get the plane of the polygon
    // get the normal vector
    assert(vertexesCount >= 3);
    Vector N = normalVectorOfConvexPolygon(self, (Point){0, 0, 0});
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
    // That the ray just lies on the plane
    // we simply count this situation as no collision
    Vector D = ray->direction;
    Vector O = ray->start;
    double C = vecDot(N, cp->vertexes[0]);
    double NdotD = vecDot(N, D);
    if (fabs(NdotD) < error) {
        return (CollideResult){false, INFINITY};
    }
    double t = (C - vecDot(N, O)) / NdotD;
    Point P = vecAdd(O, vecMultiply(t, D));
    // Now we have converted this problem into if a point is in the convex polygon
    Point lastPoint = cp->vertexes[vertexesCount - 1];
    Vector lastE = vecMinus(P, lastPoint);
    Vector lastE1 = vecMinus(cp->vertexes[0], lastPoint);
    Vector firstResult = vecCross(lastE, lastE1);

    for (int i = 0; i < vertexesCount - 1; i++) {
        // Since it's an convex polygon, we see if the point is on the left side of every edge
        Point P1 = cp->vertexes[i];
        Point P2 = cp->vertexes[i + 1];
        // E is the vector from start point to intersection P
        Vector E = vecMinus(P, P1);
        // E1 is the polygon's ith edge
        Vector E1 = vecMinus(P2, P1);
        Vector crossResult = vecCross(E, E1);
        if (vecDot(crossResult, firstResult) < 0) {
            return (CollideResult){false, INFINITY};
        }
    }
    return (CollideResult){true, t};
}

static void destroyConvexPolygon(Shape *self) {
    ConvexPolygon *cp = (ConvexPolygon*)self;
    freeDynamicArray(cp->vertexes);
    free(self);
}

Shape *newConvexPolygon(Point *vertexes) {
    ConvexPolygon *cp = malloc(sizeof(*cp));
    cp->shape = (Shape){collideWithConvexPolygon, normal}
}