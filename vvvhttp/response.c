#include <errno.h>

#include "internal.h"

int vvvhttp_response_set_header_string(struct vvvhttp_response * res,
                                       char const * key, size_t key_len,
                                       char const * value, size_t value_len)
{
    struct vstring_pair * h;
    if (res->headers_count == ARRAY_SIZE(res->headers)) {
        return -ENOSPC;
    }

    h = &res->headers[res->headers_count++];

    h->first.ptr = key;
    h->first.len = key_len;
    h->second.ptr = value;
    h->second.len = value_len;

    return 0;
}
