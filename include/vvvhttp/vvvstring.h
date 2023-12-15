#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct vstring
{
    char const * ptr;
    int len;
};

struct vstring_pair
{
    struct vstring first;
    struct vstring second;
};

static inline int vstring_equal(struct vstring const * a, struct vstring const * b)
{
    return a->len == b->len && memcmp(a->ptr, b->ptr, a->len) == 0;
}
