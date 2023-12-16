#pragma once

#include "vvvstring.h"
#include <stdbool.h>

#define VVVHTTP_ROUTE_MAX_SEGMENTS      64
#define VVVHTTP_REQUEST_MAX_HEADERS     64
#define VVVHTTP_REQUEST_MAX_PARAMS      64
#define VVVHTTP_RESPONSE_MAX_HEADERS    64

#ifndef ARRAY_SIZE
#   define ARRAY_SIZE(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#endif

struct vvvhttp_response;
struct vvvhttp_request;

typedef int (*vvvhttp_route_handler)(struct vvvhttp_request const * req,
                                  struct vvvhttp_response * res);
typedef void (*vvvhttp_cleanup_callback)(struct vvvhttp_response * res);
typedef enum vvvhttp_processor_status (*vvvhttp_processor_callback)(struct vvvhttp_request * req,
                                                                    struct vvvhttp_response * res);

enum vvvhttp_method
{
    HTTP_METHOD_CONNECT     = (1 << 0),
    HTTP_METHOD_DELETE      = (1 << 1),
    HTTP_METHOD_GET         = (1 << 2),
    HTTP_METHOD_HEAD        = (1 << 3),
    HTTP_METHOD_OPTIONS     = (1 << 4),
    HTTP_METHOD_POST        = (1 << 5),
    HTTP_METHOD_PUT         = (1 << 6),
    HTTP_METHOD_TRACE       = (1 << 7),
};

enum vvvhttp_processor_status
{
    /** Allow the request to pass through to the next processor or to the route handler */
    VVVHTTP_PROCESSOR_PASS = 0,
    /** Reject the request with no response */
    VVVHTTP_PROCESSOR_REJECT,
    /** Respond to the request with processor-provided response, does not enter route handler */
    VVVHTTP_PROCESSOR_RESPONSE,
};

struct vvvhttp_request
{
    enum vvvhttp_method method;
    unsigned http_version;
    struct vstring path;
    size_t headers_count;
    struct vstring_pair headers[VVVHTTP_REQUEST_MAX_HEADERS];
    size_t params_count;
    struct vstring_pair params[VVVHTTP_REQUEST_MAX_PARAMS];
    struct vstring body;
    struct vstring content_type;
    /** Unused by vvvhttp, free to be used in user code */
    void * userdata;
};

struct vvvhttp_response
{
    unsigned http_version;
    unsigned code;
    size_t headers_count;
    struct vstring_pair headers[VVVHTTP_RESPONSE_MAX_HEADERS];
    struct vstring body;
    struct vstring content_type;
    vvvhttp_cleanup_callback cleanup_callback;
    /** Unused by vvvhttp, free to be used in user code */
    void * userdata;
};

struct vvvhttp_route_cache
{
    bool initialized;
    struct vstring route_segments[VVVHTTP_ROUTE_MAX_SEGMENTS];
};

struct vvvhttp_route
{
    struct vstring route;
    unsigned supported_methods;
    vvvhttp_route_handler request_handler;
    vvvhttp_cleanup_callback cleanup_callback;

    size_t const preprocessors_count;
    vvvhttp_processor_callback const * preprocessors;

    size_t const postprocessors_count;
    vvvhttp_processor_callback const * postprocessors;

    struct vvvhttp_route_cache * route_cache;

    /** Unused by vvvhttp, free to be used in user code */
    void * userdata;
};

#define VVVHTTP_PROCESSOR_LIST(...) __VA_ARGS__

#define _VVVHTTP_DEFINE_ROUTE_1(_id, \
                                _preprocessors, _preprocessors_count, \
                                _postprocessors, _postprocessors_count, \
                                _route, _supported_methods, \
                                _request_handler, _cleanup_callback, _userdata) \
    static struct vvvhttp_route_cache __vvvhttp_route_cache_##_id = { 0 }; \
    static struct vvvhttp_route const __vvvhttp_route_##_id \
    = { \
        .route = { \
            .ptr = _route, \
            .len = sizeof(_route) - 1, \
        }, \
        .supported_methods = _supported_methods, \
        .request_handler = _request_handler, \
        .cleanup_callback = _cleanup_callback, \
        .preprocessors_count = _preprocessors_count, \
        .preprocessors = _preprocessors, \
        .postprocessors_count = _postprocessors_count, \
        .postprocessors = _postprocessors, \
        .route_cache = &__vvvhttp_route_cache_##_id, \
        .userdata = _userdata, \
    }; \
    static struct vvvhttp_route const * const _vvvhttp_route_ptr_##_id \
        __attribute((__section__("vvvhttp_routes"))) \
        __attribute__((__used__)) \
        = &__vvvhttp_route_##_id

#define _VVVHTTP_PASTE_TOKEN_1(a, b) a ## b
#define _VVVHTTP_PASTE_TOKEN(a, b) _VVVHTTP_PASTE_TOKEN_1(a, b)

#define VVVHTTP_DEFINE_ROUTE(_id, _preprocessors, _postprocessors, \
                             _route, _supported_methods, \
                             _request_handler, _cleanup_callback, _userdata) \
    static vvvhttp_processor_callback const __vvvhttp_route_preprocessors_##_id[] = \
        _preprocessors; \
    static vvvhttp_processor_callback const __vvvhttp_route_postprocessors_##_id[] = \
        _postprocessors; \
    _VVVHTTP_DEFINE_ROUTE_1(_id, __vvvhttp_route_preprocessors_##_id, \
                          ARRAY_SIZE(__vvvhttp_route_preprocessors_##_id), \
                          __vvvhttp_route_postprocessors_##_id, \
                          ARRAY_SIZE(__vvvhttp_route_postprocessors_##_id), \
                          _route, _supported_methods, _request_handler, \
                          _cleanup_callback, _userdata)

/**
 * Initialize routes, should be called once before passing any reuqests to vvvhttp.
 *
 * @return 0 on success, <0 on error.
 */
int vvvhttp_init(void);

static inline void vvvhttp_init_response(struct vvvhttp_response * res,
                                         int http_version_major, int http_version_minor)
{
    memset(res, 0, sizeof(*res));
    res->http_version = http_version_major * 10 + http_version_minor;
}

/**
 * Parse request data
 *
 * @param request Request structure to be parsed into
 * @param data Request data to be parsed
 * @param data_len Number of bytes to parse in @ref data
 * @return 0 on success, <0 on error.
 */
int vvvhttp_parse_request(struct vvvhttp_request * req, char const * data, size_t data_len);

/**
 * Serialize response into bytes
 *
 * @param res Response data to be serialized
 * @param data Serialization output buffer
 * @param data_len Number of bytes available in @ref data
 * @return 0 on success, <0 on error.
 */
int vvvhttp_serialize_response(struct vvvhttp_response const * res, char * data, size_t data_len);

/**
 * Route a request
 *
 * @param target_route Route into which the request was routed into
 * @param req Incoming request
 * @param res Response structure to write the route response into
 * @return 0 on success, <0 on error.
 */
int vvvhttp_route(struct vvvhttp_request * req, struct vvvhttp_response * res);

/**
 * Run route cleanup actions
 *
 * @param route Route that served the response
 * @param res Response data
 */
static inline void vvvhttp_cleanup(struct vvvhttp_response * res)
{
    if (res->cleanup_callback) {
        res->cleanup_callback(res);
    }
}

/* Utility functions */

static inline void vvvhttp_response_set_code(struct vvvhttp_response * res, int code)
{
    res->code = code;
}

static inline void vvvhttp_response_set_body(struct vvvhttp_response * res,
                                             char const * data, size_t len)
{
    res->body.ptr = data;
    res->body.len = len;
}

static inline void vvvhttp_response_set_content_type_cstring(struct vvvhttp_response * res,
                                                             char const * content_type)
{
    res->content_type.ptr = content_type;
    res->content_type.len = strlen(content_type);
}

static inline void vvvhttp_response_set_content_type_string(struct vvvhttp_response * res,
                                                     char const * content_type, size_t len)
{
    res->content_type.ptr = content_type;
    res->content_type.len = len;
}

int vvvhttp_response_set_header_string(struct vvvhttp_response * res,
                                       char const * key, size_t key_len,
                                       char const * value, size_t value_len);

static inline int vvvhttp_response_set_header_cstring(struct vvvhttp_response * res,
                                               char const * key, char const * value)
{
    return vvvhttp_response_set_header_string(res, key, strlen(key), value, strlen(value));
}

static inline void vvvhttp_response_set_cleanup_callback(struct vvvhttp_response * res,
                                                         vvvhttp_cleanup_callback callback)
{
    res->cleanup_callback = callback;
}

/**
 * Error handler
 * When responding with resources that need cleanup then this function must set the response
 * cleanup callback itself.
 *
 * @param req Incoming request
 * @param res Buffer for response
 * @return VVVHTTP_PROCESSOR_RESPONSE if provides a response to respond with,
 *         anything else to drop connection
 */
enum vvvhttp_processor_status vvvhttp_error_handler(struct vvvhttp_request * req,
                                                    struct vvvhttp_response * res);
