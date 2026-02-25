#ifndef __HYPERLOG_H__
#define __HYPERLOG_H__

#include "printf.h"

#define LOG_INFO(...) printf("[Mini-Hyper info] " __VA_ARGS__)
#define LOG_WARN(...) printf("[Mini-Hyper warn] " __VA_ARGS__)

#endif
