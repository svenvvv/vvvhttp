#include "internal.h"
#include "vvvlog.h"

__attribute__((__weak__))
int vvvlog_handler(enum vvvlog_level level, char const * fmt, ...)
{
    (void)level;
    (void)fmt;
    return 0;
}

__attribute__((__weak__))
enum vvvhttp_processor_status vvvhttp_error_handler(struct vvvhttp_request * req,
                                                    struct vvvhttp_response * res)
{
    (void)res;
    LOG_ERR("Error handling route \"%.*s\", code %d\n", req->path.len, req->path.ptr);
    return VVVHTTP_PROCESSOR_REJECT;
}

