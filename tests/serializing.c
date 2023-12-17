#include "common.h"

#define DEF_REQ(method, uri, httpver) \
( \
    method " " uri " HTTP/" httpver "\r\n" \
    "Host: localhost:8080\r\n" \
    "User-Agent: test/1.0.0\r\n" \
    "Accept: */*\r\n" \
    "\r\n" \
)

#define TEST(name, expected, ctor) \
    static int name(void) \
    { \
        int err; \
        char out_buf[2048]; \
        struct vvvhttp_response res; \
        ctor \
        err = vvvhttp_serialize_response(&res, out_buf, sizeof(out_buf)); \
        if (err <= 0) { \
            return err; \
        } \
        printf("out (%d): %.*s\n", err, err, out_buf); \
        if (memcmp(out_buf, expected, err) != 0) { \
            return 1; \
        } \
        return 0; \
    }

// HTTP version
TEST(test_http_version_1_1,
    "HTTP/1.1 200 OK\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 200);
    }
);
TEST(test_http_version_2_0,
    "HTTP/2.0 200 OK\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 2, 0);
        vvvhttp_response_set_code(&res, 200);
    }
);

// Methods
TEST(test_response_200,
    "HTTP/1.1 200 OK\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 200);
    }
);
TEST(test_response_201,
    "HTTP/1.1 201 Created\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 201);
    }
);
TEST(test_response_301,
    "HTTP/1.1 301 Moved Permanently\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 301);
    }
);
TEST(test_response_400,
    "HTTP/1.1 400 Bad Request\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 400);
    }
);
TEST(test_response_404,
    "HTTP/1.1 404 Not Found\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 404);
    }
);
TEST(test_response_500,
    "HTTP/1.1 500 Internal Server Error\r\n"
    "Server: vvvhttp\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 500);
    }
);

// Headers
TEST(test_headers_single,
    "HTTP/1.1 200 OK\r\n"
    "Server: vvvhttp\r\n"
    "Key: Value\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 200);
        err = vvvhttp_response_set_header_cstring(&res, "Key", "Value");
        if (err != 0) {
            return err;
        }
    }
);
TEST(test_headers_multiple,
    "HTTP/1.1 200 OK\r\n"
    "Server: vvvhttp\r\n"
    "Key: Value\r\n"
    "Second: Something\r\n"
    "\r\n",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 200);
        err = vvvhttp_response_set_header_cstring(&res, "Key", "Value");
        if (err != 0) {
            return err;
        }
        err = vvvhttp_response_set_header_cstring(&res, "Second", "Something");
        if (err != 0) {
            return err;
        }
    }
);

// Body
TEST(test_body,
    "HTTP/1.1 200 OK\r\n"
    "Server: vvvhttp\r\n"
    "Content-Length: 9\r\n"
    "\r\n"
    "body data",
     {
        vvvhttp_init_response(&res, 1, 1);
        vvvhttp_response_set_code(&res, 200);
        vvvhttp_response_set_body(&res, "body data", sizeof("body data") - 1);
    }
);

test_fn const tests[] = {
    test_http_version_1_1,
    test_http_version_2_0,

    test_response_200,
    test_response_201,
    test_response_301,
    test_response_400,
    test_response_404,
    test_response_500,

    test_headers_single,
    test_headers_multiple,

    test_body,
};
size_t tests_count = ARRAY_SIZE(tests);
