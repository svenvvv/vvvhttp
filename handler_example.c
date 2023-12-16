#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <vvvhttp/vvvhttp.h>
#include <vvvhttp/vvvlog.h>

#define ARG_UNUSED(x) ((void)x)

void root_route_cleanup(struct vvvhttp_response * res)
{
    if (res->body.ptr != NULL) {
        free((void*)res->body.ptr);
    }
}

int root_route_handler(struct vvvhttp_request const * req, struct vvvhttp_response * res)
{
    ARG_UNUSED(req);

    vvvhttp_response_set_code(res, 200);

    char * body = malloc(128);
    if (body == NULL) {
        return -ENOMEM;
    }

    for (int i = 0; i < 128; ++i) {
        body[i] = '1' + (i % 9);
    }
    vvvhttp_response_set_body(res, body, 128);

    return 0;
}

enum vvvhttp_processor_status cache_preprocessor(struct vvvhttp_request * req,
                                                 struct vvvhttp_response * res)
{
    ARG_UNUSED(req);
    ARG_UNUSED(res);

    printf("auth interceptor 1\n");
    return 0;
}

enum vvvhttp_processor_status cache_postprocessor(struct vvvhttp_request * req,
                                                  struct vvvhttp_response * res)
{
    ARG_UNUSED(req);
    ARG_UNUSED(res);

    printf("auth interceptor 2\n");
    return 0;
}

VVVHTTP_DEFINE_ROUTE(first,
    VVVHTTP_PROCESSOR_LIST({ cache_preprocessor }),
    VVVHTTP_PROCESSOR_LIST({ cache_postprocessor }),
    "/test/mega/long/route/(\\d+)/", HTTP_METHOD_GET | HTTP_METHOD_POST,
    root_route_handler, root_route_cleanup, NULL);

VVVHTTP_DEFINE_ROUTE(second,
    VVVHTTP_PROCESSOR_LIST({}),
    VVVHTTP_PROCESSOR_LIST({}),
    "/", HTTP_METHOD_GET | HTTP_METHOD_POST,
    root_route_handler, root_route_cleanup, NULL);

int vvvlog_handler(enum vvvlog_level level, char const * fmt, ...)
{
    int err;
    FILE * outfd;
    va_list va;
    char const * level_str;

    if (level >= VVVLOG_LEVEL_WRN) {
        outfd = stderr;
    } else {
        outfd = stdout;
    }

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

    fprintf(outfd, "[vvvhttp:%s]:", level_str);

    va_start(va, fmt);
    err = vfprintf(outfd, fmt, va);
    va_end(va);

    return err;
}

void error_cleanup_callback(struct vvvhttp_response * res)
{
    if (res->body.ptr) {
        free((void*)res->body.ptr);
    }
}

enum vvvhttp_processor_status vvvhttp_error_handler(struct vvvhttp_request * req,
                                                    struct vvvhttp_response * res)
{
#define ERROR_404_BODY "<body><h1>404 Page not found</h1><p>Unknown route: %.*s</p></body>\n"

    // Over allocates some bytes due to the format string
    size_t buf_sz = sizeof(ERROR_404_BODY) + req->path.len;
    char * buf = malloc(buf_sz);
    if (buf == NULL) {
        return VVVHTTP_PROCESSOR_REJECT;
    }
    int body_sz = snprintf(buf, buf_sz, ERROR_404_BODY, req->path.len, req->path.ptr);

    vvvhttp_response_set_code(res, 404);
    vvvhttp_response_set_body(res, buf, body_sz);
    vvvhttp_response_set_cleanup_callback(res, error_cleanup_callback);

    return VVVHTTP_PROCESSOR_RESPONSE;

#undef ERROR_404_BODY
}
