#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "internal.h"
#include "vvvlog.h"

static int parse_http_method(enum vvvhttp_method * res, char const * data, size_t data_len)
{
    __ASSERT(res != NULL);
    __ASSERT(data != NULL);

    if (data_len == 0) {
        goto invalid;
    }

    switch (data_len) {
        case sizeof("CONNECT") - 1: /* sizeof("OPTIONS") - 1 */
            if (memcmp(data, "CONNECT", sizeof("CONNECT") - 1) == 0) {
                *res = HTTP_METHOD_CONNECT;
            } else if (memcmp(data, "OPTIONS", sizeof("OPTIONS") - 1) == 0) {
                *res = HTTP_METHOD_OPTIONS;
            } else {
                goto invalid;
            }
            break;
        case sizeof("DELETE") - 1:
            if (memcmp(data, "DELETE", sizeof("DELETE") - 1) == 0) {
                *res = HTTP_METHOD_DELETE;
            } else {
                goto invalid;
            }
            break;
        case sizeof("GET") - 1: /* sizeof("PUT") - 1 */
            if (memcmp(data, "GET", sizeof("GET") - 1) == 0) {
                *res = HTTP_METHOD_GET;
            } else if (memcmp(data, "PUT", sizeof("PUT") - 1) == 0) {
                *res = HTTP_METHOD_PUT;
            } else {
                goto invalid;
            }
            break;
        case sizeof("HEAD") - 1: /* sizeof("POST") - 1 */
            if (memcmp(data, "HEAD", sizeof("HEAD") - 1) == 0) {
                *res = HTTP_METHOD_HEAD;
            } else if (memcmp(data, "POST", sizeof("POST") - 1) == 0) {
                *res = HTTP_METHOD_POST;
            } else {
                goto invalid;
            }
            break;
        case sizeof("TRACE") - 1:
            if (memcmp(data, "TRACE", sizeof("TRACE") - 1) == 0) {
                *res = HTTP_METHOD_TRACE;
            } else {
                goto invalid;
            }
            break;
        default:
            goto invalid;
    }

    return 0;
invalid:
    return -EINVAL;
}

static char const * find_newline(char const * s, ssize_t n)
{
    __ASSERT(s != NULL);

    if (n < 0) {
        return NULL;
    }

    char const * ret = memchr(s, '\r', n);
    if (ret != NULL && ret - s < n - 1 && ret[1] == '\n') {
        return ret;
    }
    return NULL;
}

static void * find_param_end(char const * s, ssize_t n)
{
    __ASSERT(s != NULL);

    for (int i = 0; i < n; ++i) {
        char ch = s[i];
        if (ch == '&' || ch == ' ') {
            return (void*)(s + i);
        }
    }
    return NULL;
}

static void * find_char_on_line(char const * s, char needle, ssize_t n)
{
    __ASSERT(s != NULL);

    for (int i = 0; i < n; ++i) {
        char ch = s[i];
        if (ch == needle) {
            return (void*)(s + i);
        }
        if (ch == '\n') {
            break;
        }
    }
    return NULL;
}

char const * parse_parameters(struct vvvhttp_request * request,
                              char const * buf, char const * buf_end)
{
    char const * param = buf;
    char const * param_end = NULL;
    char const * param_mid = NULL;

    for (int param_num = 1;; ++param_num) {
        struct vstring_pair * p = &request->params[request->params_count];

        if (param >= buf_end) {
            LOG_ERR("param: malformed param %d\n", param_num);
            goto error;
        }

        param_end = find_param_end(param, buf_end - param);
        if (param_end == NULL) {
            LOG_ERR("param: malformed, no end for param %d\n", param_num);
            goto error;
        }

        param_mid = find_char_on_line(param, '=', param_end - param);
        if (param_mid == NULL) {
            p->first.ptr = param;
            p->first.len = param_end - param;
            p->second.len = 0;

            LOG_DBG("param: \"%.*s\" = no value\n", p->first.len, p->first.ptr);
        } else {
            p->first.ptr = param;
            p->first.len = param_mid - param;
            p->second.ptr = param_mid + 1;
            p->second.len = param_end - param_mid - 1;

            LOG_DBG("param: \"%.*s\" = \"%.*s\"\n",
                    p->first.len, p->first.ptr, p->second.len, p->second.ptr);
        }

        if (*param_end == ' ') {
            param = param_end;
            break;
        }
        param = param_end + 1;
    }

error:
    return param;
}

bool is_number(char ch)
{
    return '0' <= ch && ch <= '9';
}

int vvvhttp_parse_request(struct vvvhttp_request * request, char const * data, size_t data_len)
{
    int err;
    char const * const buf_end = data + data_len;

    char const * substr = data;
    char const * substr_end = find_char_on_line(substr, ' ', buf_end - substr);
    if (substr_end == NULL) {
        goto invalid;
    }
    size_t substr_len = substr_end - substr;

    err = parse_http_method(&request->method, substr, substr_len);
    if (err != 0) {
        goto invalid;
    }
    LOG_DBG("method %d to %.*s\n", substr_len, substr_len, substr);

    substr = substr_end + 1;
    substr_end = find_char_on_line(substr, '?', buf_end - substr);
    if (substr_end == NULL) {
        request->params_count = 0;
        substr_end = find_char_on_line(substr, ' ', buf_end - substr);
        if (substr_end == NULL) {
            goto invalid;
        }
        substr_len = substr_end - substr;
    } else {
        substr_len = substr_end - substr;
        substr_end = parse_parameters(request, substr_end + 1, buf_end - substr_len);
    }
    request->path.ptr = substr;
    request->path.len = substr_len;

    substr = substr_end + 1;
    substr_end = find_newline(substr, buf_end - substr);
    if (substr_end == NULL) {
        goto invalid;
    }
    substr_len = substr_end - substr;

    if (substr_len < sizeof("HTTP/") - 1 || memcmp(substr, "HTTP/", sizeof("HTTP/") - 1) != 0) {
        LOG_ERR("parse: malformed HTTP version\n");
        goto invalid;
    }
    substr += sizeof("HTTP/") - 1;
    substr_len = substr_end - substr;

    if (!is_number(*substr) || !is_number(*(substr + 2))) {
        LOG_ERR("parse: malformed HTTP version\n");
        goto invalid;
    }
    request->http_version = (((*substr) - '0') * 10) + (*(substr + 2) - '0');

    LOG_DBG("HTTP version: %.*s, = %d\n", substr_len, substr, request->http_version);

    request->headers_count = 0;
    for (;;) {
        struct vstring_pair * next_header = NULL;

        substr = substr_end + 2;
        substr_end = find_char_on_line(substr, ':', buf_end - substr);
        if (substr_end != NULL) {
            substr_len = substr_end - substr;

            if (request->headers_count == ARRAY_SIZE(request->headers)) {
                LOG_ERR("Too many headers, unable to parse\n");
                goto invalid;
            }
            next_header = &request->headers[request->headers_count++];

            next_header->first.ptr = substr;
            next_header->first.len = substr_len;

            substr_end += 1;
        } else {
            substr_end = substr - 1;
        }

        substr = substr_end + 1;
        substr_end = find_newline(substr, buf_end - substr);
        if (substr_end == NULL) {
            goto invalid;
        }
        substr_len = substr_end - substr;

        if (substr_len == 0) {
            substr_end += 2;
            break;
        }

        if (next_header == NULL) {
            LOG_ERR("Malformed header value?\n");
            goto invalid;
        }

        next_header->second.ptr = substr;
        next_header->second.len = substr_len;

        LOG_DBG("header: \"%.*s\" = \"%.*s\"\n",
                next_header->first.len, next_header->first.ptr,
                next_header->second.len, next_header->second.ptr);
    }

    request->body.ptr = substr_end;
    request->body.len = buf_end - substr_end;
    LOG_DBG("body (%u): \"%.*s\"\n", buf_end - substr_end, buf_end - substr_end, substr_end);
    LOG_DBG("req to %.*s\n", request->path.len, request->path.ptr);

    return 0;
invalid:
    return -EINVAL;
}
