#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <vvvhttp/vvvhttp.h>
#include <vvvhttp/vvvlog.h>

#define ARG_UNUSED(x) ((void)x)

#define EXPECT_NULL(expected) \
    do { \
        if ((expected) != NULL) { \
            fprintf(stderr, "Failed: %p != NULL, %s:%d\n", (expected), __FILE__, __LINE__); \
            return 1; \
        } \
    } while (false)

#define EXPECT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "Failed: %d != %d, %s:%d\n", \
                    (int)(expected), (int)(actual), __FILE__, __LINE__); \
            return 1; \
        } \
    } while (false)

#define EXPECT_EQ_VSTR(expected, actual) \
    do { \
        int exlen = strlen((expected)); \
        if (exlen != (actual).len && memcmp((expected), (actual).ptr, exlen) != 0) { \
            fprintf(stderr, "Failed: %s != %.*s, %s:%d\n", \
                    (expected), (actual).len, (actual).ptr, __FILE__, __LINE__); \
            return 1; \
        } \
    } while (false)

typedef int (*test_fn)(void);

extern test_fn const tests[];
extern size_t tests_count;
