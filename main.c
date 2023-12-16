#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <vvvhttp/vvvhttp.h>

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
