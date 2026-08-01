#ifndef FRAME_H 
#define FRAME_H 
#include "shapes.h"

typedef struct Frame Frame;

Frame *readFrame(const char *path);

Shape *newGroupFromFrame(Frame *m, double scalingFactor, Point center);
#endif
