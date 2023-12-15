#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "vvvhttp.h"
#include "vvvlog.h"

void root_route_cleanup(struct vvvhttp_response * res)
{
    free((void*)res->body.ptr);
}

int root_route_handler(struct vvvhttp_request const * req, struct vvvhttp_response * res)
{
    (void)req;

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
    (void)req;
    (void)res;

    printf("auth interceptor 1\n");
    return 0;
}

enum vvvhttp_processor_status cache_postprocessor(struct vvvhttp_request * req,
                                                  struct vvvhttp_response * res)
{
    (void)req;
    (void)res;

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

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int err;
    int fd;
    int opt = 1;
    struct sockaddr_in server_addr;

    err = vvvhttp_init();
    if (err != 0) {
        fprintf(stderr, "Failed to init vvvhttp, %d\n", err);
        exit(EXIT_FAILURE);
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "Failed to open socket, %d", errno);
        exit(EXIT_FAILURE);
    }

    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (err != 0) {
        fprintf(stderr, "Failed to set sockopt, %d", errno);
        goto error;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);

    err = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (err != 0) {
        fprintf(stderr, "Failed to bind(), %d", errno);
        goto error;
    }

    err = listen(fd, 10);
    if (err != 0) {
        fprintf(stderr, "Failed to listen(), %d", errno);
        goto error;
    }

    for (;;) {
        int childfd;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        char buf[4096];
        struct vvvhttp_request request;
        struct vvvhttp_response response = { 0 };

        childfd = accept(fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (childfd < 0) {
            fprintf(stderr, "Failed to accept(), %d", errno);
            goto invalid;
        }

        ssize_t const buf_len = read(childfd, buf, sizeof(buf));
        if (buf_len < 0) {
            fprintf(stderr, "Failed to read(), %d", errno);
            goto invalid;
        } else if (buf_len == 0) {
            fprintf(stderr, "Received no data from peer");
            goto invalid;
        }

        printf("recv %li:\n%s", buf_len, buf);

        err = vvvhttp_parse_request(&request, buf, buf_len);
        if (err != 0) {
            fprintf(stderr, "Failed to parse request, %d\n", err);
            goto invalid;
        }

        vvvhttp_init_response(&response, 1, 0);

        err = vvvhttp_route(&request, &response);
        if (err != 0) {
            fprintf(stderr, "Failed to route request, %d\n", err);
            goto invalid;
        }

        err = vvvhttp_serialize_response(&response, buf, sizeof(buf));
        if (err <= 0) {
            fprintf(stderr, "Failed to route request, %d\n", err);
            goto invalid;
        }

        err = write(childfd, buf, err);
        if (err < 0) {
            fprintf(stderr, "Failed to send(), %d\n", errno);
            goto invalid;
        }

        vvvhttp_cleanup(&response);
        continue;
invalid:
        vvvhttp_cleanup(&response);
        close(childfd);
    }

    close(fd);
    exit(EXIT_SUCCESS);
error:
    close(fd);
    exit(EXIT_FAILURE);
}

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
    free((void*)res->body.ptr);
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
