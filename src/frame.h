#ifndef FRAME_H 
#define FRAME_H 
#include "shapes.h"
#include "transform.h"

typedef struct Frame Frame;

Frame *readFrame(const char *path);

Shape *newGroupFromFrame(Frame *m, Transform t, void *transformPara);
#endif
