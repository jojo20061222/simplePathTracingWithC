#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <assert.h>
#include <stdlib.h>

/*
 * @author seaofvoices
 * @brief a template of dynamic array which is similar to std::vector
 */

typedef struct {
    int capacity, size;
} dynamicArrayHeader;

#define newDynamicArray(arraySize, type)                                                           \
    ({                                                                                             \
        assert((arraySize) >= 0);                                                                  \
        dynamicArrayHeader *__header =                                                             \
            malloc(((arraySize) + 1) * sizeof(type) + sizeof(*__header));                          \
        __header->capacity = (arraySize) + 1;                                                      \
        __header->size = (arraySize);                                                              \
        (type *)(__header + 1);                                                                    \
    })

#define pushBack(pointerToArray, x, type)                                                          \
    do {                                                                                           \
        type *__array = *(pointerToArray);                                                         \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(__array)) - 1;                      \
        __header->size++;                                                                          \
        if (__header->size > __header->capacity) {                                                 \
            __header->capacity *= 2;                                                               \
            __header = realloc(__header, __header->capacity * sizeof(type) + sizeof(*__header));   \
            *(pointerToArray) = (type *)(__header + 1);                                            \
            (__array) = (type *)(__header + 1);                                                    \
            assert(__header != NULL);                                                              \
        }                                                                                          \
        (__array)[__header->size - 1] = (x);                                                       \
    } while (0)

#define popBack(array)                                                                             \
    do {                                                                                           \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(array)) - 1;                        \
        __header->size--;                                                                          \
    } while (0)

#define getSize(array)                                                                             \
    ({                                                                                             \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(array)) - 1;                        \
        __header->size;                                                                            \
    })

#define getCapacity(array)                                                                         \
    ({                                                                                             \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(array)) - 1;                        \
        __header->capacity;                                                                        \
    })

#define getBack(array)                                                                             \
    ({                                                                                             \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(array)) - 1;                        \
        (array)[__header->size - 1];                                                               \
    })

#define freeDynamicArray(array)                                                                    \
    do {                                                                                           \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(array)) - 1;                        \
        free(__header);                                                                            \
    } while (0)

#define reserve(pointerToArray, reservedSize, type)                                                \
    do {                                                                                           \
        type *__array = *(pointerToArray);                                                         \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(__array)) - 1;                      \
        if (__header->capacity < (reservedSize)) {                                                 \
            __header->capacity = (reservedSize);                                                   \
            __header = realloc(__header, __header->capacity * sizeof(type) + sizeof(*__header));   \
            *(pointerToArray) = (type *)(__header + 1);                                            \
            (__array) = (type *)(__header + 1);                                                    \
        }                                                                                          \
        assert(__header != NULL);                                                                  \
    } while (0)

#define resize(pointerToArray, newSize, type)                                                      \
    do {                                                                                           \
        type *__array = *(pointerToArray);                                                         \
        dynamicArrayHeader *__header = ((dynamicArrayHeader *)(__array)) - 1;                      \
        if ((newSize) > __header->capacity) {                                                      \
            reserve((pointerToArray), (newSize), type);                                            \
            __array = *(pointerToArray);                                                           \
            __header = ((dynamicArrayHeader *)(__array)) - 1;                                      \
        }                                                                                          \
        __header->size = (newSize);                                                                \
    } while (0)

#endif