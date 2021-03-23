#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"

void printdf(unsigned level, const char* format, ...){
    if(level <= DEBUG_LOG_LEVEL){
        va_list args;
        va_start(args, format);

        //ideally would like to print out timestamp as well, will look into
        switch(level){
            case DEBUG_ERR:
                printf("-E- ");
                break;
            case DEBUG_WARN:
                printf("-W- ");
                break;
            case DEBUG_DEBUG:
                printf("-D- ");
                break;
            case DEBUG_INFO:
                printf("-I- ");
                break;
        }

        vprintf(format, args);
        va_end(args);
    }
}

