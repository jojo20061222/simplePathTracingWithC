#include "multithreads.h"
#include "random.h"
#include "render.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int leftBoundary, rightBoundary;
} Range;

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *rear;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    unsigned int length; // avoid a lot of confising situation
} ThreadSafeQueue;

static ThreadSafeQueue *newTSQueue() {
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

    tsq->length = 0;

    return tsq;
}

// after using the TSQueue we destroy the queue
// Note that this function will also free the data in the queue
static void destroyTSQueue(ThreadSafeQueue *tsq) {
    // destroy condition variable
    pthread_cond_destroy(&tsq->cond);
    // destroy mutex
    pthread_mutex_destroy(&tsq->mutex);
    // free all the element in the queue
    for (int i = 0; i < tsq->length; i++) {
        Node *temp = tsq->head;
        tsq->head = tsq->head->next;
        free(temp->data);
        free(temp);
    }
    free(tsq);
}

static inline bool TSQIsEmpty(ThreadSafeQueue *tsq) { return tsq->length == 0; }

// push the data into the queue
static void TSQPush(ThreadSafeQueue *tsq, void *data) {
    // lock the mutex to avoid race condition
    pthread_mutex_lock(&tsq->mutex);

    // add an element to the rear of the linked list
    Node *temp = malloc(sizeof(*temp));
    temp->data = data;
    temp->next = NULL;

    if (tsq->length == 0) {
        // the queue is originally empty
        tsq->head = temp;
        tsq->rear = temp;
    }
    else {
        tsq->rear->next = temp;
        tsq->rear = temp;
    }

    ++tsq->length;

    pthread_mutex_unlock(&tsq->mutex);

    // after push an element into the queue, the queue is no longer empty
    // so, we need to notify a thread to work
    pthread_cond_signal(&tsq->cond);
}

// pop out data from queue and return the poped out data
static void *TSQPop(ThreadSafeQueue *tsq) {
    // lock the mutex to avoid race condition
    pthread_mutex_lock(&tsq->mutex);

    // wait until there is an element to be poped
    while (TSQIsEmpty(tsq)) {
        pthread_cond_wait(&tsq->cond, &tsq->mutex);
    }

    // now we should have locked up the mutex so do the following operations
    // pop out the head node from the linked list
    Node *temp = tsq->head;
    tsq->head = tsq->head->next;
    --tsq->length;
    void *resultData = temp->data;
    free(temp);
    // data should be freed by the owner after poping out
    pthread_mutex_unlock(&tsq->mutex);

    return resultData;
}

static inline bool inBoundary(int var, int limit) { return var >= 0 && var < limit; }

typedef struct {
    int id;
    VirtualScreen *vs;
    Object **objects;
    LightSource **sources;
    Point *camera;
    int workerCount;
    int maxDepth;
    int *exit;
    ThreadSafeQueue *taskQueue;
} WorkerPara;

typedef struct Multithreads {
    pthread_t *threadPool;
    ThreadSafeQueue *taskQueue;
    int threadCount;
    int *exit;
} Multithreads;

static void *worker(void *para) {
    WorkerPara *workerPara = (WorkerPara *)para;
    int id = workerPara->id;
    Object **objects = workerPara->objects;
    LightSource **sources = workerPara->sources;
    Point *camera = workerPara->camera;
    int maxDepth = workerPara->maxDepth;
    VirtualScreen *vs = workerPara->vs;
    ThreadSafeQueue *taskQueue = workerPara->taskQueue;

    int width = getScreenWidth(vs);
    int height = getScreenHeight(vs);
    int rangeLength = width * height;

    MT19937 *mt = initMT19937(id * 42 + time(NULL) + 19650218U);

    // the worker will update the status of pixels every (blockSize) times of loop
    int blockSize = 64;

    while (*workerPara->exit == 0) {
        // here we get the data from the task queue
        // don't forget to recycle it later :)
        Range *range = TSQPop(taskQueue);
        int leftBoundary = range->leftBoundary;
        int rightBoundary = range->rightBoundary;

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
            Real RE = getRelativeError(vs, x, y);
            if (RE < 5e-2) {
                *accessConvergent(vs, x, y) = true;
            }
        }

        TSQPush(taskQueue, range);
    }

    destroyMT19937(mt);
    free(para);

    return NULL;
}

Multithreads *initMultithreads(VirtualScreen *vs, int threadCount, Shape **shapes,
                               Texture **textures, Object **objects, LightSource **sources,
                               Point *camera, int maxDepth, int sampleThreshold) {
    Multithreads *mul = malloc(sizeof(Multithreads));
    pthread_t *threadPool = malloc(sizeof(pthread_t) * threadCount);

    mul->exit = malloc(sizeof(int));
    if (mul->exit == NULL) {
        fprintf(stderr, "malloc result->exit failed\n");
        exit(1);
    }
    *mul->exit = 0;

    int rangeLength = getScreenWidth(vs) * getScreenHeight(vs);
    // set render threshold for every pixel
    bool *touchMap = getTouchMap(camera, vs, objects, sources);
    setThreshold(vs, touchMap, sampleThreshold);

    // here, we divide the screen into (blockCount) blocks
    // and push them into task queue for workers to work
    int blockCount = threadCount * 2;
    ThreadSafeQueue *taskQueue = newTSQueue();
    mul->taskQueue = taskQueue;

    for (int id = 0; id < blockCount; id++) {
        // given id of the block, we devide the range [0, width * height - 1] into (blockCount)
        // pieces
        // so that every block is allocated a piece
        int leftBoundary = (rangeLength / blockCount) * id;
        int rightBoundary = (rangeLength / blockCount) * (id + 1) - 1;
        if (id == blockCount - 1) rightBoundary = rangeLength - 1;
        Range *range = malloc(sizeof(*range));
        *range = (Range){leftBoundary, rightBoundary};
        TSQPush(taskQueue, range);
    }

    // create threads
    for (int i = 0; i < threadCount; i++) {
        WorkerPara *parai = malloc(sizeof(WorkerPara));
        if (parai == NULL) {
            *mul->exit = true;
            for (int j = 0; j < i - 1; j++)
                pthread_join(mul->threadPool[j], NULL);
            fprintf(stderr, "malloc para%d failed\n", i);
            exit(1);
        }
        // set parameters for every threads
        parai->id = i;
        parai->objects = objects;
        parai->sources = sources;
        parai->vs = vs;
        parai->workerCount = threadCount;
        parai->camera = camera;
        parai->maxDepth = maxDepth;
        parai->exit = mul->exit;
        parai->taskQueue = taskQueue;
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
    destroyTSQueue(mul->taskQueue);
    free(mul);
}