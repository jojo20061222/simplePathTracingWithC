#include "shapes.h"
#include "base.h"
#include <math.h>
#include <stdlib.h>

CollideResult detectTriangleCollide(const struct _Shape *self, const ReversedRay *ray) {
    CollideResult result = {false, INFINITY};
    /*
     * Möller-Trumbore intersection algorithm
     * Parametrize the triangle with T(u, v) and the points lies in the triangle can be expressed as
     * T(u, v) = uA + vB + P, u + v <= 1, 0 <= u, v <= 1 where A, B, P represent two side vector and
     * one vertex, respectively.
     * Then, let line be L(t) = O + tD we have O + tD = uA + vB + P. Since there are three unknowns
     * and three equations t, u, v can thus be solved.
     */

    const Triangle *tri = (const Triangle *)self;
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
Vector normalVectorOfTriangle(const struct _Shape *self, Point _) {
    const Triangle *tri = (const Triangle *)self;
    return vecCross(vecMinus(tri->p2, tri->p1), vecMinus(tri->p3, tri->p1));
}

Triangle newTriangle(Point p1, Point p2, Point p3) {
    return (Triangle){(Shape){detectTriangleCollide, normalVectorOfTriangle}, p1, p2, p3};
}

CollideResult detectSphereCollide(const struct _Shape *self, const ReversedRay *ray) {
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
Vector normalVectorOfSphere(const struct _Shape *self, Point p) {
    const Sphere *sph = (const Sphere *)self;
    return vecMinus(p, sph->center);
}

Sphere newSphere(Point center, double radius) {
    return (Sphere){(Shape){detectSphereCollide, normalVectorOfSphere}, center, radius};
}