
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <vvvhttp/vvvhttp.h>
#include <vvvhttp/vvvlog.h>

#define ARG_UNUSED(x) ((void)x)

#define DEF_REQ(method, uri, httpver) \
( \
    method " " uri " HTTP/" httpver "\r\n" \
    "Host: localhost:8080\r\n" \
    "User-Agent: test/1.0.0\r\n" \
    "Accept: */*\r\n" \
    "\r\n" \
)

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

#define TEST(name, input_data, checks) \
    static int name(void) \
    { \
        int err; \
        struct vvvhttp_request request; \
        char const * input = input_data; \
        err = vvvhttp_parse_request(&request, input, strlen(input)); \
        if (err != 0) { \
            return err; \
        } \
        checks \
        return 0; \
    }

char const * uri_generator_indexed_query_params(int count)
{
    static char uri[4 * 1024];
    static char buf[4 * 1024 + 128];
    char * head = uri;
    char * head_end = uri + ARRAY_SIZE(uri);
    size_t ret;

    ret = snprintf(head, head_end - head, "/?key0=value0");
    if (ret <= 0) {
        return NULL;
    }
    head += ret;

    for (int i = 1; i < count; ++i) {
        size_t ret = snprintf(head, head_end - head, "&key%d=value%d", i, i);
        if (ret <= 0) {
            break;
        }
        head += ret;
    }

    ret = snprintf(buf, sizeof(buf), DEF_REQ("GET", "%s", "1.1"), uri);
    if (ret <= 0) {
        return NULL;
    }

    return buf;
}

// Query params

TEST(test_query_params_none, DEF_REQ("GET", "/", "1.1"), {
    EXPECT_EQ(0, request.params_count);
});

TEST(test_query_params_val, DEF_REQ("GET", "/?first=value1", "1.1"), {
    EXPECT_EQ(1, request.params_count);
    EXPECT_EQ_VSTR("first", request.params[0].first);
    EXPECT_EQ_VSTR("value1", request.params[0].second);
});

TEST(test_query_params_val_with_url, DEF_REQ("GET", "/an-url-before-params?first=value1", "1.1"), {
    EXPECT_EQ(1, request.params_count);
    EXPECT_EQ_VSTR("first", request.params[0].first);
    EXPECT_EQ_VSTR("value1", request.params[0].second);
    EXPECT_EQ_VSTR("/an-url-before-params", request.path);
});

TEST(test_query_params_val_noval_val, DEF_REQ("GET", "/?first=value1&second&third=value3", "1.1"), {
    EXPECT_EQ(3, request.params_count);
    EXPECT_EQ_VSTR("first", request.params[0].first);
    EXPECT_EQ_VSTR("value1", request.params[0].second);
    EXPECT_EQ_VSTR("second", request.params[1].first);
    EXPECT_NULL(request.params[1].second.ptr);
    EXPECT_EQ(0, request.params[1].second.len);
    EXPECT_EQ_VSTR("third", request.params[2].first);
    EXPECT_EQ_VSTR("value3", request.params[2].second);
});

TEST(test_query_params_max, uri_generator_indexed_query_params(VVVHTTP_ROUTE_MAX_SEGMENTS), {
    EXPECT_EQ(VVVHTTP_ROUTE_MAX_SEGMENTS, request.params_count);
});

// HTTP version

TEST(test_http_version_1_1, DEF_REQ("GET", "/", "1.1"), {
    EXPECT_EQ(11, request.http_version);
});
TEST(test_http_version_2_0, DEF_REQ("GET", "/", "2.0"), {
    EXPECT_EQ(20, request.http_version);
});

// Methods

TEST(test_method_connect, DEF_REQ("CONNECT", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_CONNECT, request.method);
});
TEST(test_method_delete, DEF_REQ("DELETE", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_DELETE, request.method);
});
TEST(test_method_get, DEF_REQ("GET", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_GET, request.method);
});
TEST(test_method_head, DEF_REQ("HEAD", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_HEAD, request.method);
});
TEST(test_method_options, DEF_REQ("OPTIONS", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_OPTIONS, request.method);
});
TEST(test_method_post, DEF_REQ("POST", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_POST, request.method);
});
TEST(test_method_put, DEF_REQ("PUT", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_PUT, request.method);
});
TEST(test_method_trace, DEF_REQ("TRACE", "/", "1.1"), {
    EXPECT_EQ(HTTP_METHOD_TRACE, request.method);
});

// Path tests
TEST(test_path_empty, DEF_REQ("GET", "/", "1.1"), {
    EXPECT_EQ_VSTR("/", request.path);
    EXPECT_EQ(1, request.path_segments_count);
    EXPECT_EQ_VSTR("", request.path_segments[0]);
});
TEST(test_path_simple, DEF_REQ("GET", "/single-path", "1.1"), {
    EXPECT_EQ_VSTR("/single-path", request.path);
    EXPECT_EQ(1, request.path_segments_count);
    EXPECT_EQ_VSTR("single-path", request.path_segments[0]);
});
TEST(test_path_multiple, DEF_REQ("GET", "/first/second/third", "1.1"), {
    EXPECT_EQ_VSTR("/first/second/third", request.path);
    EXPECT_EQ(3, request.path_segments_count);
    EXPECT_EQ_VSTR("first", request.path_segments[0]);
    EXPECT_EQ_VSTR("second", request.path_segments[1]);
    EXPECT_EQ_VSTR("third", request.path_segments[2]);
});

typedef int (*test_fn)(void);

int main(int argc, char ** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    int err;

    test_fn tests[] = {
        test_query_params_none,
        test_query_params_val,
        test_query_params_val_with_url,
        test_query_params_val_noval_val,
        test_query_params_max,

        test_http_version_1_1,
        test_http_version_2_0,

        test_method_connect,
        test_method_delete,
        test_method_get,
        test_method_head,
        test_method_options,
        test_method_post,
        test_method_put,
        test_method_trace,

        test_path_empty,
        test_path_simple,
        test_path_multiple,
    };

    for (size_t i = 0; i < ARRAY_SIZE(tests); ++i) {
        err = tests[i]();
        if (err != 0) {
            fprintf(stderr, "Test %zu failed with code %d\n", i + 1, err);
            return -1;
        }
    }

    return 0;
}

int vvvlog_handler(enum vvvlog_level level, char const * fmt, ...)
{
    int err;
    va_list va;
    char const * level_str;

    switch (level) {
        case VVVLOG_LEVEL_DBG:
            level_str = "dbg";
            break;
        case VVVLOG_LEVEL_INF:
            level_str = "inf";
            break;
        case VVVLOG_LEVEL_WRN:
            level_str = "wrn";
            break;
        case VVVLOG_LEVEL_ERR:
            level_str = "err";
            break;
        default:
            level_str = "unk";
            break;
    }

    fprintf(stdout, "[vvvhttp:%s]:", level_str);

    va_start(va, fmt);
    err = vfprintf(stdout, fmt, va);
    va_end(va);

    return err;
}

VVVHTTP_DEFINE_ROUTE(none,
    VVVHTTP_PROCESSOR_LIST({}),
    VVVHTTP_PROCESSOR_LIST({}),
    "/", HTTP_METHOD_GET,
    NULL, NULL, NULL);
