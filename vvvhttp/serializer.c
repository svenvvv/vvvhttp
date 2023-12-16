#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "internal.h"
#include "vvvlog.h"

static int serialize_response_code_string(int code, char * data, size_t data_len)
{
    char const * str;
    size_t str_len = 0;

    switch (code) {
        case 100:
            str = "Continue";
            str_len = sizeof("Continue") - 1;
            break;
        case 101:
            str = "Switching Protocols";
            str_len = sizeof("Switching Protocols") - 1;
            break;
        case 102:
            str = "Processing";
            str_len = sizeof("Processing") - 1;
            break;
        case 103:
            str = "Early Hints";
            str_len = sizeof("Early Hints") - 1;
            break;

        case 200:
            str = "OK";
            str_len = sizeof("OK") - 1;
            break;
        case 201:
            str = "Created";
            str_len = sizeof("Created") - 1;
            break;
        case 202:
            str = "Accepted";
            str_len = sizeof("Accepted") - 1;
            break;
        case 203:
            str = "Non-Authoritative Information";
            str_len = sizeof("Non-Authoritative Information") - 1;
            break;
        case 204:
            str = "No Content";
            str_len = sizeof("No Content") - 1;
            break;
        case 205:
            str = "Reset Content";
            str_len = sizeof("Reset Content") - 1;
            break;
        case 206:
            str = "Partial Content";
            str_len = sizeof("Partial Content") - 1;
            break;
        case 207:
            str = "Multi-Status";
            str_len = sizeof("Multi-Status") - 1;
            break;
        case 208:
            str = "Already Reported";
            str_len = sizeof("Already Reported") - 1;
            break;
        case 226:
            str = "IM Used";
            str_len = sizeof("IM Used") - 1;
            break;

        case 300:
            str = "Multiple Choices";
            str_len = sizeof("Multiple Choices") - 1;
            break;
        case 301:
            str = "Moved Permanently";
            str_len = sizeof("Moved Permanently") - 1;
            break;
        case 302:
            str = "Found";
            str_len = sizeof("Found") - 1;
            break;
        case 303:
            str = "See Other";
            str_len = sizeof("See Other") - 1;
            break;
        case 304:
            str = "Not Modified";
            str_len = sizeof("Not Modified") - 1;
            break;
        case 305:
            str = "Use Proxy";
            str_len = sizeof("Use Proxy") - 1;
            break;
        case 306:
            str = "Switch Proxy";
            str_len = sizeof("Switch Proxy") - 1;
            break;
        case 307:
            str = "Temporary Redirect";
            str_len = sizeof("Temporary Redirect") - 1;
            break;
        case 308:
            str = "Permanent Redirect";
            str_len = sizeof("Permanent Redirect") - 1;
            break;

        case 400:
            str = "Bad Request";
            str_len = sizeof("Bad Request") - 1;
            break;
        case 401:
            str = "Unauthorized";
            str_len = sizeof("Unauthorized") - 1;
            break;
        case 402:
            str = "Payment Required";
            str_len = sizeof("Payment Required") - 1;
            break;
        case 403:
            str = "Forbidden";
            str_len = sizeof("Forbidden") - 1;
            break;
        case 404:
            str = "Not Found";
            str_len = sizeof("Not Found") - 1;
            break;
        case 405:
            str = "Method Not Allowed";
            str_len = sizeof("Method Not Allowed") - 1;
            break;
        case 406:
            str = "Not Acceptable";
            str_len = sizeof("Not Acceptable") - 1;
            break;
        case 407:
            str = "Proxy Authentication Required";
            str_len = sizeof("Proxy Authentication Required") - 1;
            break;
        case 408:
            str = "Request Timeout";
            str_len = sizeof("Request Timeout") - 1;
            break;
        case 409:
            str = "Conflict";
            str_len = sizeof("Conflict") - 1;
            break;
        case 410:
            str = "Gone";
            str_len = sizeof("Gone") - 1;
            break;
        case 411:
            str = "Length Required";
            str_len = sizeof("Length Required") - 1;
            break;
        case 412:
            str = "Precondition Failed";
            str_len = sizeof("Precondition Failed") - 1;
            break;
        case 413:
            str = "Payload Too Large";
            str_len = sizeof("Payload Too Large") - 1;
            break;
        case 414:
            str = "URI Too Long";
            str_len = sizeof("URI Too Long") - 1;
            break;
        case 415:
            str = "Unsupported Media Type";
            str_len = sizeof("Unsupported Media Type") - 1;
            break;
        case 416:
            str = "Range Not Satisfiable";
            str_len = sizeof("Range Not Satisfiable") - 1;
            break;
        case 417:
            str = "Expectation Failed";
            str_len = sizeof("Expectation Failed") - 1;
            break;
        case 418:
            str = "I'm a teapot";
            str_len = sizeof("I'm a teapot") - 1;
            break;
        case 421:
            str = "Misdirected Request";
            str_len = sizeof("Misdirected Request") - 1;
            break;
        case 422:
            str = "Unprocessable Entity";
            str_len = sizeof("Unprocessable Entity") - 1;
            break;
        case 423:
            str = "Locked";
            str_len = sizeof("Locked") - 1;
            break;
        case 424:
            str = "Failed Dependency";
            str_len = sizeof("Failed Dependency") - 1;
            break;
        case 425:
            str = "Too Early";
            str_len = sizeof("Too Early") - 1;
            break;
        case 426:
            str = "Upgrade Required";
            str_len = sizeof("Upgrade Required") - 1;
            break;
        case 428:
            str = "Precondition Required";
            str_len = sizeof("Precondition Required") - 1;
            break;
        case 429:
            str = "Too Many Requests ";
            str_len = sizeof("Too Many Requests ") - 1;
            break;
        case 431:
            str = "Request Header Fields Too Large";
            str_len = sizeof("Request Header Fields Too Large") - 1;
            break;
        case 451:
            str = "Unavailable For Legal Reasons";
            str_len = sizeof("Unavailable For Legal Reasons") - 1;
            break;

        case 500:
            str = "Internal Server Error";
            str_len = sizeof("Internal Server Error") - 1;
            break;
        case 501:
            str = "Not Implemented";
            str_len = sizeof("Not Implemented") - 1;
            break;
        case 502:
            str = "Bad Gateway";
            str_len = sizeof("Bad Gateway") - 1;
            break;
        case 503:
            str = "Service Unavailable";
            str_len = sizeof("Service Unavailable") - 1;
            break;
        case 504:
            str = "Gateway Timeout";
            str_len = sizeof("Gateway Timeout") - 1;
            break;
        case 505:
            str = "HTTP Version Not Supported";
            str_len = sizeof("HTTP Version Not Supported") - 1;
            break;
        case 506:
            str = "Variant Also Negotiates";
            str_len = sizeof("Variant Also Negotiates") - 1;
            break;
        case 507:
            str = "Insufficient Storage";
            str_len = sizeof("Insufficient Storage") - 1;
            break;
        case 508:
            str = "Loop Detected";
            str_len = sizeof("Loop Detected") - 1;
            break;
        case 510:
            str = "Not Extended";
            str_len = sizeof("Not Extended") - 1;
            break;
        case 511:
            str = "Network Authentication Required";
            str_len = sizeof("Network Authentication Required") - 1;
            break;

        case 218:
            str = "This is fine";
            str_len = sizeof("This is fine") - 1;
            break;

        case 419:
            str = "Page Expired";
            str_len = sizeof("Page Expired") - 1;
            break;
        case 420:
            str = "Method Failure";
            str_len = sizeof("Method Failure") - 1;
            break;
        case 430:
            str = "Request Header Fields Too Large";
            str_len = sizeof("Request Header Fields Too Large") - 1;
            break;
        case 450:
            str = "Blocked by Windows Parental Controls";
            str_len = sizeof("Blocked by Windows Parental Controls") - 1;
            break;
        case 498:
            str = "Invalid Token";
            str_len = sizeof("Invalid Token") - 1;
            break;
        case 499:
            str = "Token Required";
            str_len = sizeof("Token Required") - 1;
            break;

        case 509:
            str = "Bandwidth Limit Exceeded";
            str_len = sizeof("Bandwidth Limit Exceeded") - 1;
            break;
        case 529:
            str = "Site is overloaded";
            str_len = sizeof("Site is overloaded") - 1;
            break;
        case 530:
            str = "Site is frozen";
            str_len = sizeof("Site is frozen") - 1;
            break;
        case 598:
            str = "Network read timeout error";
            str_len = sizeof("Network read timeout error") - 1;
            break;
        case 599:
            str = "Network Connect Timeout Error";
            str_len = sizeof("Network Connect Timeout Error") - 1;
            break;

    }

    if (str_len > 0) {
        __ASSERT(str != NULL);

        if (str_len > data_len) {
            return -ENOBUFS;
        }

        memcpy(data, str, str_len);
    }

    return str_len;
}

static inline int serialize_header(char * head, size_t len,
                                   int kl, char const * k,
                                   int vl, char const * v)
{
    return snprintf(head, len, "%.*s: %.*s\r\n", kl, k, vl, v);
}

static inline int serialize_header_int(char * head, size_t len,
                                       int kl, char const * k,
                                       int value)
{
    return snprintf(head, len, "%.*s: %i\r\n", kl, k, value);
}

int vvvhttp_serialize_response(struct vvvhttp_response const * res, char * data, size_t data_len)
{
    int err;
    char * const data_end = data + data_len;
    char * head = data;

    err = snprintf(head, data_end - head, "HTTP/%u.%u %u ",
                   res->http_version / 10, res->http_version % 10, res->code);
    if (err <= 0) {
        goto error;
    }
    head += err;

    err = serialize_response_code_string(res->code, head, data_end - head);
    if (err < 0) {
        goto error;
    }
    head += err;

    if (data_end - head < 2) {
        goto error;
    }
    head[0] = '\r';
    head[1] = '\n';
    head += 2;

#if defined(VVVHTTP_RESPONSE_SERVER_ID)
    err = serialize_header(head, data_end - head,
                           sizeof("Server") - 1, "Server",
                           sizeof(VVVHTTP_RESPONSE_SERVER_ID) - 1, VVVHTTP_RESPONSE_SERVER_ID);
    if (err <= 0) {
        goto error;
    }
    head += err;
#endif

    if (res->body.len > 0) {
        err = serialize_header_int(head, data_end - head,
                                   sizeof("Content-Length") - 1, "Content-Length",
                                   res->body.len);
        if (err <= 0) {
            goto error;
        }
        head += err;

        if (res->content_type.len > 0) {
            err = serialize_header(head, data_end - head,
                                   sizeof("Content-Type") - 1, "Content-Type",
                                   res->content_type.len, res->content_type.ptr);
            if (err <= 0) {
                goto error;
            }
            head += err;
        }
    }

    for (size_t i = 0; i < res->headers_count; ++i) {
        struct vstring_pair const * header = &res->headers[i];

        __ASSERT(header->first.ptr != NULL && header->first.len > 0);
        __ASSERT(header->second.ptr != NULL && header->second.len > 0);

        err = serialize_header(head, data_end - head,
                               header->first.len, header->first.ptr,
                               header->second.len, header->second.ptr);
        if (err <= 0) {
            goto error;
        }
        head += err;
    }

    if (data_end - head < 2) {
        goto error;
    }
    head[0] = '\r';
    head[1] = '\n';
    head += 2;

    if (res->body.len > 0) {
        __ASSERT(res->body.ptr != NULL);

        if (res->body.len > data_end - head) {
            LOG_ERR("Not enough space to fit response body, got %u, need %u",
                    data_end - head, res->body.len);
            goto error;
        }

        memcpy(head, res->body.ptr, res->body.len);
        head += res->body.len;
    }

    LOG_DBG("serialized req:\n%.*s", head - data, data);
    return head - data;
error:
    LOG_ERR("Failed to serialize response, %d\n", err);
    return -1;
}
