/*
 provide log interface
 */
#include <stdio.h>
#include <stdlib.h>

#ifndef CLOG_H
#define CLOG_H

#define linfo(...) do {\
    printf("[INFO]  "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
} while(0)

#define lerror(...) do {\
    printf("[ERROR] "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
} while(0)

#define lerror_exit(...) do {\
    printf("[ERROR] "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
    exit(-1); \
} while(0)

#endif
