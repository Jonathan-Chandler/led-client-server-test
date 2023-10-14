#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>

typedef enum 
{
  ERROR_NONE,
  ERROR_BAD_PARAMETER,
  ERROR_COUNT
} ERROR_T;

typedef enum 
{
  DEBUG_OFF,
  DEBUG_ERROR,
  DEBUG_NOTICE,
  DEBUG_VERBOSE,
  DEBUG_MODE_COUNT
} debug_mode_t;

//#define DEBUG_ENABLED 1
extern debug_mode_t debug_mode;

#define dbg_verbose(...) do { \
    if (debug_mode >= DEBUG_VERBOSE) \
    { \
        fprintf(stderr, "%s::%d - %s(VERBOSE) - ",__FILE__,__LINE__,__FUNCTION__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
} while (0)

#define dbg_notice(...) do { \
    if (debug_mode >= DEBUG_NOTICE) \
    { \
        fprintf(stderr, "%s::%d - %s(NOTICE) - ",__FILE__,__LINE__,__FUNCTION__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
} while (0)

#define dbg_error(...) do { \
    if (debug_mode >= DEBUG_ERROR) \
    { \
        fprintf(stderr, "%s::%d - %s(ERROR) - ",__FILE__,__LINE__,__FUNCTION__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
} while (0)

#endif //__DEBUG_H__
