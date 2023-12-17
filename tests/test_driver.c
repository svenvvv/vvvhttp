#include "common.h"

int main(int argc, char ** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    int err;

    for (size_t i = 0; i < tests_count; ++i) {
        err = tests[i]();
        if (err != 0) {
            fprintf(stderr, "Test %zu failed with code %d\n", i + 1, err);
            return -1;
        }
    }

    return 0;
}

int vvvlog_handler(enum vvvlog_level level, char const * fmt, ...)
{
    int err;
    va_list va;
    char const * level_str;

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

    fprintf(stdout, "[vvvhttp:%s]:", level_str);

    va_start(va, fmt);
    err = vfprintf(stdout, fmt, va);
    va_end(va);

    return err;
}

VVVHTTP_DEFINE_ROUTE(none,
    VVVHTTP_PROCESSOR_LIST({}),
    VVVHTTP_PROCESSOR_LIST({}),
    "/", HTTP_METHOD_GET,
    NULL, NULL, NULL);

