#ifndef __HYPERLOG_H__
#define __HYPERLOG_H__

#include "printf.h"
#include "types.h"
#include "timer.h"

// #define LOG_INFO(...) printf("[Mini-Hyper info] " __VA_ARGS__)
// #define LOG_WARN(...) printf("[Mini-Hyper warn] " __VA_ARGS__)

#if 0

#define LOG_INFO(fmt, ...) do { \
    u64 _us = timer_get_us(); \
    u32 _sec = _us / 1000000; \
    u32 _usec = _us % 1000000; \
    printf("[%5u.%06u][Hyper-Info] " fmt, _sec, _usec, ##__VA_ARGS__); \
} while(0)

#define LOG_WARN(fmt, ...) do { \
    u64 _us = timer_get_us(); \
    u32 _sec = _us / 1000000; \
    u32 _usec = _us % 1000000; \
    printf("[%5u.%06u][Hyper-Warn] " fmt, _sec, _usec, ##__VA_ARGS__); \
} while(0)

#endif

#if 1

#define SEC5_TO_CHARS(sec) \
    ('0' + ((sec) / 10000) % 10), \
    ('0' + ((sec) / 1000) % 10), \
    ('0' + ((sec) / 100) % 10), \
    ('0' + ((sec) / 10) % 10), \
    ('0' + ((sec) % 10))

#define LOG_INFO(fmt, ...) do { \
    u64 _us = timer_get_us(); \
    u32 _sec = _us / 1000000; \
    u32 _usec = _us % 1000000; \
    printf("[%c%c%c%c%c.%06u][Hyper-Info] " fmt, \
           SEC5_TO_CHARS(_sec), _usec, ##__VA_ARGS__); \
} while(0)

#define LOG_WARN(fmt, ...) do { \
    u64 _us = timer_get_us(); \
    u32 _sec = _us / 1000000; \
    u32 _usec = _us % 1000000; \
    printf("[%c%c%c%c%c.%06u][Hyper-Warn] " fmt, \
           SEC5_TO_CHARS(_sec), _usec, ##__VA_ARGS__); \
} while(0)

#endif

#endif
