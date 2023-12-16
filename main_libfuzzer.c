#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vvvhttp/vvvhttp.h>

int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t buf_len)
{
    int err;
    struct vvvhttp_request request;
    struct vvvhttp_response response = { 0 };

    size_t data_len = 4096;
    if (data_len < buf_len) {
        data_len = buf_len;
    }
    char * data = malloc(data_len);
    if (!data) {
        return -1;
    }
    memcpy(data, buf, buf_len);

    err = vvvhttp_parse_request(&request, data, buf_len);
    if (err != 0) {
        fprintf(stderr, "Failed to parse request, %d\n", err);
        goto exit;
    }

    vvvhttp_init_response(&response, 1, 0);

    err = vvvhttp_route(&request, &response);
    if (err != 0) {
        fprintf(stderr, "Failed to route request, %d\n", err);
        vvvhttp_cleanup(&response);
        goto exit;
    }

    err = vvvhttp_serialize_response(&response, data, data_len);
    if (err <= 0) {
        fprintf(stderr, "Failed to route request, %d\n", err);
        vvvhttp_cleanup(&response);
        goto exit;
    }

exit:
    free(data);
    return 0;
}
