#include "frame.h"
#include "base.h"
#include "dynamicArray.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int *Face;

typedef struct Frame {
    Point *vertexes;
    Face *faces;
} Frame;

Frame *readFrame(const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        printf("Open file %s failed\n", path);
        exit(1);
    }

    Frame *frame = malloc(sizeof(*frame));
    frame->faces = newDynamicArray(0, Face);
    frame->vertexes = newDynamicArray(0, Point);

    char *buffer = NULL;
    size_t bufferSize = 0;

    while (getline(&buffer, &bufferSize, f) != -1) {
        // split the line
        char *saveptr = NULL;
        static const char dim[] = " ";
        char *firstTok = strtok_r(buffer, dim, &saveptr);

        if (!strcmp(firstTok, "v")) { // first case: this line represent a vertex
            Point p;
            sscanf(buffer + 2, "%lf%lf%lf", &p.x, &p.y, &p.z); // skip the "v " at the start
#ifdef DEBUG
            printf("read vertex: %lf %lf %lf\n", p.x, p.y, p.z);
#endif
        }

        else if (!strcmp(firstTok, "f")) { // second case: this line represent a face
            bool lineEnd = false;
            Face f = newDynamicArray(0, int);
#ifdef DEBUG
            printf("read face: ");
#endif
            while (!lineEnd) {
                char *token = strtok_r(NULL, dim, &saveptr);
                if (!token) {
                    lineEnd = true;
                    break;
                }
                int vertexNumber, _;
                sscanf(token, "%d//%d", &vertexNumber, &_);
#ifdef DEBUG
                printf("%d ", vertexNumber);
#endif
                pushBack(&f,
                         vertexNumber - 1, // in obj files they are 1-based but we need 0-based here
                         int);
            }
#ifdef DEBUG
            printf("\n");
#endif
            pushBack(&frame->faces, f, Face);
        }
    }

    free(buffer);
    fclose(f);
    return frame;
}

Shape *newGroupFromFrame(Frame *m, double scalingFactor, Point center) {
    Shape **shapes = newDynamicArray(0, Shape *);
    int faceCount = getSize(m->faces);
    for (int i = 0; i < faceCount; i++) {
#ifdef DEBUG
        printf("f: ");
#endif 
        int vertexesCount = getSize(m->faces[i]);
        Point *vertexes = newDynamicArray(0, Point);
        for (int j = 0; j < vertexesCount; j++) {
            int vertexNumber = m->faces[i][j];
            Point p = m->vertexes[vertexNumber];
            p = vecAdd(vecMultiply(scalingFactor, p), center);
#ifdef DEBUG
            printf("(%lf, %lf, %lf) ", p.x, p.y, p.z);
#endif
            pushBack(&vertexes, p, Point);
        }
#ifdef DEBUG
        printf("\n");
#endif
        Shape *cp = newConvexPolygon(vertexes);
        pushBack(&shapes, cp, Shape *);
        freeDynamicArray(vertexes);
    }
    return newGroup(shapes);
}