#include "shapes.h"
#include "base.h"
#include <math.h>
#include <stdlib.h>
static const double error = 1e-8;

CollisionResult detectLineCollide(const struct _Shape *self, const ReversedRay *ray) {
    CollisionResult result;
    // some math
    /*
     * Here we shall calculate the distance between lines. This is done by solving the equation
     * listed.
     * Let P(t) = S + tA, Q(r) = T + rB be a point on lines, A, B be directions of lines.
     * (P(t) - Q(r)) dot A = (P(t) - Q(r)) dot B = 0
     * Since there is two equations with two unknowns, t and r can be solved consequently.
     * Expanding the equation we get:
     * (S + tA - T - rB) dot A = 0 -> A^2 t - (A dot B) r = (T - S) dot A
     * (S + tA - T - rB) dot B = 0 -> (A dot B) t - B^2 r = (T - S) dot B
     * Simply replace the coefficient and we get
     * at - br = d
     * bt - cr = e
     * where d = W dot A, e = W dot B
     */

    const LineSegment *line = (const LineSegment *)self;
    Vector S = line->start;
    Vector A = minus(line->end, line->start);
    Vector T = ray->start;
    Vector B = ray->direction;
    Vector W = minus(T, S);

    double a = dot(A, A);
    double b = dot(A, B);
    double c = dot(B, B);
    double d = dot(W, A);
    double e = dot(W, B);

    // Applying Cramer's Rule to solve the equation

    double delta = b * b - a * c; // delta < 0
    double t, r;
    if (-delta < error) {
        /*
         * cos(theta)^2 approx 1 => two lines are parallel
         * since they are parallel we choose any point on the segment (t = 0 here)
         */
        t = 0;
        // to get the best result we use the larger denominator
        r = fabs(b) > fabs(c) ? -d / b : -e / c;
    }
    else {
        double deltat = b * e - d * c;
        double deltar = a * e - d * b;
        t = deltat / delta;
        r = deltar / delta;
    }

    // t might goes beyond the range 0 <= t <= 1 so we must adjust that
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    r = (b * t - e) / c;

    // we do same thing on r
    if (r < 0.0) {
        r = 0.0;
        t = d / a;
        if (t < 0.0) t = 0.0;
        if (t > 1.0) t = 1.0;
    }

    // finally we calculate the distance of two points
    Point Pt = plus(S, scalerMultiplication(t, A));
    Point Qr = plus(T, scalerMultiplication(r, B));
    Vector N = minus(Pt, Qr);
    double distance = sqrt(dot(N, N));

    if (distance > line->width) {
        result.collide = false;
    }
    else {
        result.collide = true;
    }
    result.t = r;
    return result;
}
// TODO: figure out how should this work since it should not work by its definition .w.
// may be it needs an incident ray for reference?
Vector lineNormalVector(const struct _Shape *self, Point targetPoint, const ReversedRay *ray) {
    return (Vector){0, 0, 0};
}

LineSegment newLineSegment(Point start, Point end, double width) {
    return (LineSegment){(Shape){detectLineCollide, lineNormalVector}, start, end, width};
}

const static struct {
    int first, second;
} CubeEdgeRelation[12] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 4}, {1, 5},
                          {2, 6}, {3, 7}, {4, 5}, {5, 6}, {6, 7}, {7, 4}};

CollisionResult detectCubeCollide(const struct _Shape *self, const ReversedRay *ray) {
    const Cube *cube = (const Cube *)self;

    CollisionResult result;
    bool collide = false;
    double minT = INFINITY;

    for (int i = 0; i < 12; i++) {
        int a = CubeEdgeRelation[i].first, b = CubeEdgeRelation[i].second;
        LineSegment line = newLineSegment(cube->points[a], cube->points[b], cube->width);
        CollisionResult curResult = detectLineCollide((const Shape *)&line, ray);
        if (curResult.collide) {
            collide = true;
            if (curResult.t < minT) {
                minT = curResult.t;
                result = curResult;
            }
        }
    }

    if (!collide) {
        result.collide = false;
    }

    return result;
}

// this function is used to temporarily set the vertexes to the situation that center is at origin
// without really changing its center
void setCubeVertexesOrigin(Cube *cube) {
    for (int i = 0; i < 8; i++) {
        cube->points[i] = minus((Vector)cube->points[i], cube->center);
    }
}

void unsetCubeVertexesOrigin(Cube *cube) {
    for (int i = 0; i < 8; i++) {
        cube->points[i] = plus((Vector)cube->points[i], cube->center);
    }
}

// TODO: to combine these three function into one integrated one (might not be possible since the
// disgusting .x .y .z) and rewrite the three function by calling the integrated one with different
// parameters rotate a Cube around Z axiz, where rotate angle is in degree
void rotateCubeZ(Cube *cube, double angle) {
    angle = angle / 180 * M_PI;
    Vector *vertexes = cube->points;

    // first we move the vertexes back to the origin
    setCubeVertexesOrigin(cube);

    // apply the rotate matrix
    for (int i = 0; i < 8; i++) {
        double x = vertexes[i].x, y = vertexes[i].y;
        vertexes[i].x = cos(angle) * x - sin(angle) * y;
        vertexes[i].y = sin(angle) * x + cos(angle) * y;
    }

    // finally, move the Cube back to its center
    unsetCubeVertexesOrigin(cube);
}

// rotate a Cube around Y axiz, where rotate angle is in degree
void rotateCubeY(Cube *cube, double angle) {
    angle = angle / 180 * M_PI;
    Vector *vertexes = cube->points;

    // first we move the vertexes back to the origin
    setCubeVertexesOrigin(cube);

    // apply the rotate matrix
    for (int i = 0; i < 8; i++) {
        double x = vertexes[i].x, z = vertexes[i].z;
        vertexes[i].x = cos(angle) * x - sin(angle) * z;
        vertexes[i].z = sin(angle) * x + cos(angle) * z;
    }

    // finally, move the Cube back to its center
    unsetCubeVertexesOrigin(cube);
}

// rotate a Cube around Y axiz, where rotate angle is in degree
void rotateCubeX(Cube *cube, double angle) {
    angle = angle / 180 * M_PI;
    Vector *vertexes = cube->points;

    // first we move the vertexes back to the origin
    setCubeVertexesOrigin(cube);

    // apply the rotate matrix
    for (int i = 0; i < 8; i++) {
        double y = vertexes[i].y, z = vertexes[i].z;
        vertexes[i].y = cos(angle) * y - sin(angle) * z;
        vertexes[i].z = sin(angle) * y + cos(angle) * z;
    }

    // finally, move the Cube back to its center
    unsetCubeVertexesOrigin(cube);
}

void moveCube(Cube *cube, Vector displacement) {
    setCubeVertexesOrigin(cube);
    cube->center = plus(cube->center, displacement);
    for (int i = 0; i < 8; i++) {
        cube->points[i] = plus(cube->points[i], cube->center);
    }
}

void setCubePosition(Cube *cube, Point pos) {
    setCubeVertexesOrigin(cube);
    cube->center = pos;
    for (int i = 0; i < 8; i++) {
        cube->points[i] = plus(cube->points[i], cube->center);
    }
}

Cube *newCube(double length, Point center, double width) {
    Cube *result = malloc(sizeof(Cube));
    double halfLength = length / 2;
    result->points = malloc(sizeof(Point) * 8);
    // set the 8 vertexes
    result->points[0] = (Point){halfLength, halfLength, halfLength};
    result->points[1] = (Point){-halfLength, halfLength, halfLength};
    result->points[2] = (Point){-halfLength, -halfLength, halfLength};
    result->points[3] = (Point){halfLength, -halfLength, halfLength};
    result->points[4] = (Point){halfLength, halfLength, -halfLength};
    result->points[5] = (Point){-halfLength, halfLength, -halfLength};
    result->points[6] = (Point){-halfLength, -halfLength, -halfLength};
    result->points[7] = (Point){halfLength, -halfLength, -halfLength};

    for (int i = 0; i < 8; i++) {
        result->points[i] = plus(result->points[i], center);
    }

    result->center = center;
    result->width = width;
    result->shape.detectCollide = detectCubeCollide;
    result->shape.reflectVector = NULL;
    return result;
}

void freeCube(Cube *Cube) {
    free(Cube->points);
    free(Cube);
}

CollisionResult detectTriangleCollide(const struct _Shape *self, const ReversedRay *ray) {
    CollisionResult result = {false, INFINITY};
    /*
     * Möller-Trumbore intersection algorithm
     * Parametrize the triangle with T(u, v) and the points lies in the triangle can be expressed as
     * T(u, v) = uA + vB + P, u + v <= 1, 0 <= u, v <= 1 where A, B, P represent two side vector and
     * one vertex, respectively.
     * Then, let line be L(t) = O + tD we have O + tD = uA + vB + P. Since there are three unknowns
     * and three equations t, u, v can thus be solved.
     */

    const Triangle *tri = (const Triangle *)self;
    Vector A = minus(tri->p2, tri->p1);
    Vector B = minus(tri->p3, tri->p1);
    Vector W = minus(ray->start, tri->p1); // let W = O - P
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

    result = (CollisionResult){true, t};

    return result;
}

Triangle newTriangle(Point p1, Point p2, Point p3) {
    return (Triangle){(Shape){detectTriangleCollide, NULL}, p1, p2, p3};
}

CollisionResult detectSphereCollide(const struct _Shape *self, const ReversedRay *ray) {
    /*
     * Some math.
     * Let R(t) = O + tD = <a + tx, b + ty, c + tz>, center of sphere C = <e, f, g>
     * <a + tx, b + ty, c + tz> - <e, f, g> = <a - e + tx, b - f + ty, c - g + tz>
     * Hence we solve for minimum of (a - e + tx)^2 + (b - f + ty)^2 + (c - g + tz)^2 = r^2
     * Let W = O - C = <o, p, q>
     * then (x^2 + y^2 + z^2)t^2 + 2(ox + py + qz)t + (o^2 + p^2 + q^2 - r^2) = 0
     */

    const Sphere *sphere = (const Sphere *)self;

    Vector W = minus(ray->start, sphere->center);
    double A = dot(ray->direction, ray->direction);
    double B = 2 * dot(ray->direction, W);
    double C = dot(W, W) - sphere->radius * sphere->radius;
    double Delta = B * B - 4 * A * C;
    if (Delta < 0) return (CollisionResult){false, INFINITY};
    double t1 = (-B - sqrt(Delta)) / (2 * A), t2 = (-B + sqrt(Delta)) / (2 * A);
    if (t1 < 0 && t2 < 0)
        return (CollisionResult){false, INFINITY};
    else if (t1 < 0)
        return (CollisionResult){true, t2};
    return (CollisionResult){true, t1};
}

Sphere newSphere(Point center, double radius) {
    return (Sphere){(Shape){detectSphereCollide, NULL}, center, radius};
}