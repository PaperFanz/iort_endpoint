/*
 * Define debug levels and provide a debug print function
 */

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>


#define DEBUG_NONE  0
#define DEBUG_ERR   1
#define DEBUG_WARN  2
#define DEBUG_DEBUG 3
#define DEBUG_INFO  4
#define DEBUG_ALL  UINT_MAX

#define DEBUG_LOG_LEVEL DEBUG_ALL
#define DEBUG 1

void printdf(unsigned level, const char* format, ...);
