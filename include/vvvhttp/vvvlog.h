#pragma once

enum vvvlog_level
{
    VVVLOG_LEVEL_DBG = 1,
    VVVLOG_LEVEL_INF = 2,
    VVVLOG_LEVEL_WRN = 3,
    VVVLOG_LEVEL_ERR = 4,
};

int vvvlog_handler(enum vvvlog_level level, char const * fmt, ...);

#define LOG_DBG(...) \
    vvvlog_handler(VVVLOG_LEVEL_DBG, __VA_ARGS__)
#define LOG_INF(...) \
    vvvlog_handler(VVVLOG_LEVEL_INF, __VA_ARGS__)
#define LOG_WRN(...) \
    vvvlog_handler(VVVLOG_LEVEL_WRN, __VA_ARGS__)
#define LOG_ERR(...) \
    vvvlog_handler(VVVLOG_LEVEL_ERR, __VA_ARGS__)
