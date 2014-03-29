/*
 * @file        OSAL_Log.c
 * @brief
 * @author
 * @version     0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define ANDROID_LOG_DEBUG   JETOMX_LOG_TRACE
#define ANDROID_LOG_INFO    JETOMX_LOG_INFO
#define ANDROID_LOG_WARN    JETOMX_LOG_WARNING
#define ANDROID_LOG_ERROR   JETOMX_LOG_ERROR
#define ANDROID_LOG_VERBOSE JETOMX_LOG_TRACE

static void __android_log_vprint(int level,
                      const char *tag, const char *format, va_list args)
{
    vfprintf(stdout, format, args);
//    if (log_get_priority() < log_get_level("I"))
        fprintf(stdout, "%s %d %s: \n", tag, level, format);
}


#include "OSAL_Log.h"


void _OSAL_Log(JETOMX_LOG_LEVEL logLevel, const char *tag, const char *msg, ...)
{
    va_list argptr;

    va_start(argptr, msg);

    switch (logLevel) {
    case JETOMX_LOG_TRACE:
        __android_log_vprint(ANDROID_LOG_DEBUG, tag, msg, argptr);
        break;
    case JETOMX_LOG_INFO:
        __android_log_vprint(ANDROID_LOG_INFO, tag, msg, argptr);
        break;
    case JETOMX_LOG_WARNING:
        __android_log_vprint(ANDROID_LOG_WARN, tag, msg, argptr);
        break;
    case JETOMX_LOG_ERROR:
        __android_log_vprint(ANDROID_LOG_ERROR, tag, msg, argptr);
        break;
    default:
        __android_log_vprint(ANDROID_LOG_VERBOSE, tag, msg, argptr);
    }

    va_end(argptr);
}
