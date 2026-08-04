#include "multithreads.h"
#include "random.h"
#include "render.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int id;
    VirtualScreen *vs;
    Object **objects;
    LightSource **sources;
    Point *camera;
    int workerCount;
    int maxDepth;
    int *exit;
} WorkerPara;

typedef struct {
    int leftBoundary, rightBoundary;
} Pair;

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *rear;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} ThreadSafeQueue;

ThreadSafeQueue *newTSQueue() {
    ThreadSafeQueue *tsq = malloc(sizeof(*tsq));
    if (tsq == NULL) {
        fprintf(stderr, "malloc thread safe queue failed\n");
        exit(1);
    }

    tsq->head = tsq->rear = NULL;

    // init mutex
    if (pthread_mutex_init(&tsq->mutex, NULL) != 0) {
        fprintf(stderr, "init mutex failed\n");
        exit(1);
    }

    // init condition variable
    if (pthread_cond_init(&tsq->cond, NULL) != 0) {
        fprintf(stderr, "init condition variable failed\n");
        exit(1);
    }

    return tsq;
}

void destroyTSQueue(ThreadSafeQueue *tsq) {
    // destroy condition variable
    pthread_cond_destroy(&tsq->cond);
    // destroy mutex
    pthread_mutex_destroy(&tsq->mutex);
    // free all the element in the queue
    while (tsq->head) {
        free(tsq->head);
        tsq->head = tsq->head->next;
    }
    
    free(tsq);
}

void TSQPush(ThreadSafeQueue *tsq, void *data) {
    
}

static inline bool inBoundary(int var, int limit) { return var >= 0 && var < limit; }

static void *worker(void *para) {
    WorkerPara *workerPara = (WorkerPara *)para;
    int id = workerPara->id;
    Object **objects = workerPara->objects;
    LightSource **sources = workerPara->sources;
    Point *camera = workerPara->camera;
    int workerCount = workerPara->workerCount;
    int maxDepth = workerPara->maxDepth;
    VirtualScreen *vs = workerPara->vs;

    int width = getScreenWidth(vs);
    int height = getScreenHeight(vs);
    int rangeLength = width * height;
    // given id of worker, we devide the range [0, width * height - 1] into (id) pieces
    // so that every worker is allocated a piece
    int leftBoundary = (rangeLength / workerCount) * id;
    int rightBoundary = (rangeLength / workerCount) * (id + 1) - 1;
    if (id == workerCount - 1) rightBoundary = width * height - 1;

    MT19937 *mt = initMT19937(id * 42 + time(NULL) + 19650218U);

    int blockSize = 200;

    for (int i = leftBoundary; i <= rightBoundary; i++) {
        int x = getX(vs, i), y = getY(vs, i);
        setThreshold(camera, vs, x, y, objects, sources);
    }

    while (*workerPara->exit == 0) {
        for (int b = 0; b < blockSize && *workerPara->exit == 0; b++) {
            for (int i = leftBoundary; i <= rightBoundary; i++) {
                // in order to reduce cache miss rate we have but to do this :(
                int x = getX(vs, i), y = getY(vs, i);
                if (!*accessConvergent(vs, x, y)) { // adaptive sampling
                    renderPixel(camera, vs, x, y, objects, sources, mt, maxDepth);
                }
            }
        }

        // update the the convergent state of pixels
        for (int i = leftBoundary; i <= rightBoundary; i++) {
            int x = getX(vs, i), y = getY(vs, i);
            if (*accessConvergent(vs, x, y) || !isThresholdAchived(vs, x, y)) continue;
            // to ensure the pixel won't convergent too early, we consider its neighbors to decide
            // if it's gonna converge
            double RE = 0;

            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (inBoundary(x + dx, width) && inBoundary(y + dy, height)) {
                        RE = fmax(RE, getRelativeError(vs, x + dx, y + dy));
                    }
                }
            }

            if (RE < 1e-2) {
                *accessConvergent(vs, x, y) = true;
            }
        }
    }

    destroyMT19937(mt);
    free(para);

    return NULL;
}

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, Object **objects,
                               LightSource **sources, Point *camera, int maxDepth) {
    Multithreads *mul = malloc(sizeof(Multithreads));
    pthread_t *threadPool = malloc(sizeof(pthread_t) * threadCount);

    mul->exit = malloc(sizeof(int));
    if (mul->exit == NULL) {
        fprintf(stderr, "malloc result->exit failed\n");
        exit(1);
    }
    *mul->exit = 0;

    for (int i = 0; i < threadCount; i++) {
        WorkerPara *parai = malloc(sizeof(WorkerPara));
        if (parai == NULL) {
            *mul->exit = true;
            for (int j = 0; j < i - 1; j++) pthread_join(mul->threadPool[j], NULL);
            fprintf(stderr, "malloc para%d failed\n", i);
            exit(1);
        }
        parai->id = i;
        parai->objects = objects;
        parai->sources = sources;
        parai->vs = vs;
        parai->workerCount = threadCount;
        parai->camera = camera;
        parai->maxDepth = maxDepth;
        parai->exit = mul->exit;
        pthread_create(&threadPool[i], NULL, worker, parai);
    }

    mul->threadPool = threadPool;
    mul->threadCount = threadCount;
    return mul;
}

void destroyMultithreads(Multithreads *mul) {
    *mul->exit = 1;
    for (int i = 0; i < mul->threadCount; i++)
        pthread_join(mul->threadPool[i], NULL);
    free(mul->threadPool);
    free(mul->exit);
    free(mul);
}