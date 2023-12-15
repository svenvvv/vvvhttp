#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "internal.h"

enum route_node_type
{
    ROUTE_NODE_STRING,
    ROUTE_NODE_WILDCARD,
};

struct route_node
{
    enum route_node_type type;
    struct vstring string;
    struct route_node * next;
    struct route_node * children;
    struct vvvhttp_route const * route;
};

size_t route_nodes_used = 0;
struct route_node route_nodes[64];

struct route_node * root;

int vvvhttp_init(void)
{
    LOG_DBG("Initializing routes...\n");

    struct vvvhttp_route const * const * rp = &VVVHTTP_ROUTE_DEFS_START;
    for(; rp < &VVVHTTP_ROUTE_DEFS_END; ++rp){
        struct vvvhttp_route const * route = *rp;
        struct vvvhttp_route_cache * cache = route->route_cache;

        if (cache->initialized) {
            LOG_WRN("Route %*.s already initialized, skipping, "
                    "are you calling vvvhttp_init multiple times?",
                    route->route.len, route->route.ptr);
            continue;
        }

        LOG_DBG("Initializing route: %s\n",  route->route);

        char const * substr = route->route.ptr;
        char const * substr_end = NULL;
        char const * route_end = route->route.ptr + route->route.len;
        for (;;) {
            substr_end = memchr(substr, '/', route_end - substr);
            if (substr_end == NULL) {
                break;
            }

            size_t segment_len = substr_end - substr;
            LOG_DBG("seg (%d), \"%.*s\"\n", segment_len, segment_len, substr);

            substr = substr_end + 1;
        }
        // for (;;) {
        //     head = memchr(head, '/', route->route.len);
        //     // LOG_DBG("segment %.*s", head,);
        // }
        // cache->initialized = true;
    }

    return 0;
}

int vvvhttp_route(struct vvvhttp_request * req, struct vvvhttp_response * res)
{
    __ASSERT(req != NULL);
    __ASSERT(res != NULL);

    int err;
    enum vvvhttp_processor_status status;

    struct vvvhttp_route const * const * rp = &VVVHTTP_ROUTE_DEFS_START;
    for(; rp < &VVVHTTP_ROUTE_DEFS_END; ++rp){
        struct vvvhttp_route const * route = *rp;
        if ((route->supported_methods & req->method) == 0) {
            continue;
        }

        if (!vstring_equal(&route->route, &req->path)) {
            continue;
        }

        for (size_t i = 0; i < route->preprocessors_count; ++i) {
            status = route->preprocessors[i](req, res);
            if (status == VVVHTTP_PROCESSOR_REJECT) {
                LOG_DBG("Pre-processor %d rejected request, not forwarding to route\n", i);
                err = -1;
                goto error;
            } else if (status == VVVHTTP_PROCESSOR_RESPONSE) {
                LOG_DBG("Pre-processor %d provided response, not forwarding to route\n", i);
                err = 0;
                goto error;
            }
        }

        LOG_DBG("Matched path %.*s\n", route->route.len, route->route.ptr);

        err = route->request_handler(req, res);
        if (err < 0) {
            LOG_ERR("Request handler reports error, %d", err);
            goto error;
        }
        res->cleanup_callback = route->cleanup_callback;

        for (size_t i = 0; i < route->postprocessors_count; ++i) {
            status = route->postprocessors[i](req, res);
            if (status == VVVHTTP_PROCESSOR_REJECT) {
                LOG_DBG("Postprocessor %d rejected, responding with error", i);
                err = -1;
                goto error;
            } else if (status == VVVHTTP_PROCESSOR_RESPONSE) {
                LOG_DBG("Postprocessor %d provided own response", i);
            }
        }

        return err;
    }

    LOG_ERR("No routes match path %.*s\n", req->path.len, req->path.ptr);
error:
    status = vvvhttp_error_handler(req, res);
    if (status == VVVHTTP_PROCESSOR_RESPONSE) {
        err = 0;
    } else {
        err = -ENOENT;
    }
    return err;
}
